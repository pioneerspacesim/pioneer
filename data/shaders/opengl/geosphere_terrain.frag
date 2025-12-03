// Copyright © 2008-2026 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#include "attributes.glsl"
#include "lib.glsl"
#include "basesphere_uniforms.glsl"
#include "rayleigh.glsl"

#define WATER_SHINE 16.0

uniform sampler2D texture0;
uniform sampler2D texture1;
in vec2 texCoord0;

uniform int NumShadows;

in float dist;

in vec3 varyingEyepos;
in vec3 varyingNormal;
in vec4 vertexColor;

#ifdef TERRAIN_WITH_LAVA
in vec4 varyingEmission;
#endif

out vec4 frag_color;

uniform float PatchDetailFrequency;

#define detailScaleHi (PatchDetailFrequency * 4.0)
#define detailScaleLo (PatchDetailFrequency * 0.5)

void main(void)
{
	vec4 hidetail = texture(texture0, texCoord0 * detailScaleHi);
	vec4 lodetail = texture(texture1, texCoord0 * detailScaleLo);

	vec3 eyeposScaled = varyingEyepos * geosphereInvRadius;
	vec3 eyenorm = normalize(varyingEyepos);
	vec3 tnorm = normalize(varyingNormal);
	vec4 diff = vec4(0.0);

	vec2 planetDist = raySphereIntersect(geosphereCenter, eyenorm, geosphereRadius);
	vec3 planetIntersect = geosphereCenter + (eyenorm * planetDist.x);

	float surfaceDist = dist * geosphereInvRadius;

	// calculate the detail texture contribution from hi and lo textures
	float hiloMix = exp(-0.004 * dist);
	float detailMix = exp(-0.001 * dist);
	vec4 detailVal = mix(lodetail, hidetail, hiloMix);
	vec4 detailMul = mix(vec4(1.0), detailVal, detailMix);

#ifdef TERRAIN_WITH_WATER
	vec4 waterSpecular = vec4(0.0);
#endif

#if (NUM_LIGHTS > 0)
	vec3 V = normalize(eyeposScaled - geosphereCenter);
	vec3 I = normalize(eyeposScaled - planetIntersect);

	float AU = 149598000000.0;
	frag_color = vec4(0.f);

	// coordinates, in planet radius
	vec4 planet = vec4(geosphereCenter, geosphereRadius);
	vec4 atmosphere = vec4(geosphereCenter, geosphereAtmosTopRad);

	for (int i=0; i<NUM_LIGHTS; ++i) {
		vec3 L = normalize(uLight[i].position.xyz);
		float uneclipsed = clamp(calcUneclipsed(eclipse, NumShadows, V, L), 0.0, 1.0);
		CalcPlanetDiffuse(diff, uLight[i].diffuse, L, tnorm, uneclipsed);

#ifdef TERRAIN_WITH_WATER
		//water only for specular
	    if (vertexColor.b > 0.05 && vertexColor.r < 0.05) {
			vec3 lightPosAU = uLight[i].position.xyz / AU;
			float intensity = 1.f / dot(lightPosAU, lightPosAU); // magic to avoid calculating length and then squaring it

			waterSpecular.xyz += calculateAtmosphereColor(planet, atmosphere, toLinear(uLight[i].diffuse), reflect(L, I), (I - geosphereCenter) * geosphereRadius, eyenorm, uneclipsed) * intensity;
			//waterSpecular.xyz += computeIncidentLight(reflect(L, I), eyenorm, I * geosphereRadius, toLinear(uLight[i].diffuse), uneclipsed) * intensity;
		}
#endif
	}

	// Use the detail value to multiply the final colour before lighting
	vec4 ambient = scene.ambient * vertexColor;
	vec4 final = vertexColor * detailMul * diff;

#ifdef TERRAIN_WITH_WATER
	vec4 waterColor = waterSpecular * 20.f;
#endif

#ifdef ATMOSPHERE
	final = ambient + final;
#else
	// add extra brightness to atmosphere-less planetoids and dim stars
	final = ambient + final * 2.0;
#endif

	frag_color =
		material.emission +
#ifdef TERRAIN_WITH_LAVA
		varyingEmission +
#endif
		final;

#ifdef ATMOSPHERE
	frag_color += diff * atmosColor;
#ifdef TERRAIN_WITH_WATER
	frag_color += diff * waterColor;
#endif
#endif //ATMOSPHERE

#else // NUM_LIGHTS > 0 -- unlit rendering - stars
	//emission is used to boost colour of stars, which is a bit odd
	frag_color = material.emission + vertexColor;
#endif
}
