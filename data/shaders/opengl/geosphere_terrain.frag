// Copyright © 2008-2020 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#include "attributes.glsl"
#include "logz.glsl"
#include "lib.glsl"
#include "eclipse.glsl"

uniform vec4 atmosColor;
// to keep distances sane we do a nearer, smaller scam. this is how many times
// smaller the geosphere has been made
uniform float geosphereRadius;
uniform float geosphereInvRadius;
uniform float geosphereAtmosTopRad;
uniform vec3 geosphereCenter;
uniform float geosphereAtmosFogDensity;
uniform float geosphereAtmosInvScaleHeight;

uniform sampler2D texture0;	// hi detail
uniform sampler2D texture1;	// lo detail
uniform sampler2D texture2;	// lookup
uniform sampler2DArray texture3; // atlas
in vec2 uv0;
in vec2 uv1;

in float dist;
uniform float detailScaleHi;
uniform float detailScaleLo;

uniform Material material;
uniform Scene scene;

in vec3 varyingFragPos;
in vec3 varyingNormal;

#ifdef TERRAIN_WITH_LAVA
in vec4 varyingEmission;
#endif

out vec4 frag_color;

// Calculate Blinn-Phong shading for a light's contribution to the fragment
vec4 CalcLightContribution(int i, vec3 V, vec3 N, vec3 geospherePos, inout vec3 specular)
{
	// All lights processed by this shader are directional lights, assumed to be
	// infinitely far away from the surface of the geosphere. As such, the light
	// direction does not vary with respect to the fragment position, and can be
	// directly passed as a uniform
	vec3 L = normalize(uLight[i].position.xyz);

	// A scalar controlling the proportion of light reaching this fragment.
	// A value of 1.0 indicates there are no bodies in the way, whereas a value
	// of 0.0 indicates that the light is totally eclipsed by another body.
	float uneclipsed = clamp(calcUneclipsed(i, geospherePos, L), 0.0, 1.0);

	// The N dot L term from lambertian shading; controls the amount of light
	// reflected to the viewer.
	// The positive part of nDotL, used to model direct lighting contribution
	float nDotL = max(dot(N, L), 0.0);
	// The negative part, used to model indirect lighting from atmospheric scattering
	// This value is 1.0 at the horizon, and falls to 0.0 at 25% past the horizon
	float term = clamp(1.0 - max(dot(N, -L), 0.0) * 4.0, 0.0, 1.0);

#ifdef ATMOSPHERE
	// XXX: why are we using the number of lights to scale the terminator's contribution and not the overall lighting?
	// A fairly physically incorrect model for calculating planetary lighting,
	// but it captures the atmospheric terminator beyond the horizon due to atmospheric inscattering
	float lighting = 0.5 * (nDotL + 0.5 * term * INV_NUM_LIGHTS);
#else
	// Just use standard lambertian diffuse for bodies without atmosphere
	float lighting = nDotL;
#endif

	//Specular reflection
	vec3 H = normalize(L + V);
	specular += pow(max(dot(N, H), 0.0), 64.0) * uLight[i].specular.xyz * INV_NUM_LIGHTS;

	// TODO: PBR lighting model

	return uLight[i].diffuse * uneclipsed * lighting;
}

void main(void)
{
	vec4 hidetail = texture(texture0, uv0 * detailScaleHi);
	vec4 lodetail = texture(texture1, uv0 * detailScaleLo);

	vec3 fragPos = varyingFragPos;
	vec3 normal = normalize(varyingNormal);
	vec4 diff = vec4(0.0);

	// calculate the detail texture contribution from hi and lo textures
	float hiloMix = exp(-0.004 * dist);
	float detailMix = exp(-0.001 * dist);
	vec4 detailVal = mix(lodetail, hidetail, hiloMix);
	vec4 detailMul = mix(vec4(1.0), detailVal, detailMix);

	vec3 specular = vec3(0.0);

	vec4 specularIntensity = vec4(0.0); // todo: sample this from a texture.

#ifdef TEXTURE0
	// LookupTexture & slope/height texture coords
	float terrainType = texture(texture2, uv1).r;
	// mul by 16 because values are 0..1 but we need them 0..15 for array indexing
	int id0 = int(terrainType * 16.0);

	#if 1
	// cheap way of limiting the visual appearance of texture tiling
	// NB: this darkens the texture input
	vec4 s1 = texture(texture3, vec3(uv0 * detailScaleLo, float(id0))).rgba;
	vec4 s2 = texture(texture3, vec3(uv0 * detailScaleLo * -0.25, float(id0))).rgba;
	vec4 color = mix(s1, s2, 0.5);
	#else
	vec4 color = texture(texture3, vec3(uv0 * detailScaleLo, float(id0))).rgba;
	#endif
#else
	vec4 color = vec4(0.313, 0.313, 0.313, 1.0);
#endif

	vec3 geospherePos = (fragPos - geosphereCenter) * geosphereInvRadius;
	vec3 V = -normalize(fragPos);
	for (int i=0; i<NUM_LIGHTS; ++i) {
		diff += CalcLightContribution(i, V, normal, geospherePos, specular);
	}

	// Use the detail value to multiply the final colour before lighting
	vec4 final = color * detailMul;

#ifdef ATMOSPHERE
	// when does the eye ray intersect atmosphere
	float atmosStart = findSphereEyeRayEntryDistance(geosphereCenter, fragPos, geosphereRadius * geosphereAtmosTopRad);
	float ldprod=0.0;
	float fogFactor=0.0;
	{
		float atmosDist = (length(fragPos) - atmosStart);

		// a&b scaled so length of 1.0 means planet surface.
		vec3 a = (atmosStart * normalize(fragPos) - geosphereCenter) * geosphereInvRadius;
		vec3 b = (fragPos - geosphereCenter) * geosphereInvRadius;
		ldprod = AtmosLengthDensityProduct(a, b, atmosColor.w*geosphereAtmosFogDensity, atmosDist, geosphereAtmosInvScaleHeight);
		fogFactor = clamp(1.5 / exp(ldprod), 0.0, 1.0);
	}

	//calculate sunset tone red when passing through more atmosphere, clamp everything.
	float atmpower = (diff.r+diff.g+diff.b) / 3.0;
	vec4 sunset = clamp(vec4(0.8, pow(atmpower,0.8), pow(atmpower,1.2), 1.0), vec4(0.0), vec4(1.0));

	vec4 final_color =
		fogFactor *
		((scene.ambient * final) + (diff * final)) + // diffuse term
		(1.0 - fogFactor) * (diff * atmosColor) +
		(vec4(specular, 1.0) * specularIntensity * sunset) +
		(0.02 - clamp(fogFactor, 0.0, 0.01)) * diff * ldprod * sunset +	      //increase fog scatter
		(pow(1.0 - pow(fogFactor,0.75), 256.0) * 0.4 * diff * atmosColor) * sunset;  //distant fog.
#else // atmosphere-less planetoids and dim stars
	vec4 final_color =
		(scene.ambient * final) +
		(diff * final * 2.0);
#endif //ATMOSPHERE

	frag_color =
		material.emission +
#ifdef TERRAIN_WITH_LAVA
		varyingEmission +
#endif
		final_color;

	SetFragDepth();
}
