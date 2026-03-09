// Copyright © 2008-2026 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#include "attributes.glsl"
#include "lib.glsl"
#include "basesphere_uniforms.glsl"

#define WATER_SHINE 16.0

uniform sampler2D texture0;
uniform sampler2D texture1;
uniform sampler2D texture2;
uniform sampler2DArray texture3;

uniform int NumShadows;
uniform float PatchDetailFrequency;

in vec3 varyingEyepos;
in vec3 varyingNormal;
in vec2 uv0;
in vec2 uv1;
in float dist;

#ifdef TERRAIN_WITH_LAVA
in vec4 varyingEmission;
#endif

out vec4 frag_color;

#define detailScaleHi (PatchDetailFrequency * 4.0)
#define detailScaleLo (PatchDetailFrequency * 0.5)

void ReadTerrainTextureMix(vec2 uv, float layer, out vec4 textureData)
{
#if 1
	// cheap way of limiting the visual appearance of texture tiling
	float scaleFactor = exp(-0.0002 * dist);
	vec4 s1 = texture(texture3, vec3(uv * detailScaleLo * -0.25, layer)).rgba;
	vec4 s2 = texture(texture3, vec3(uv * detailScaleLo, layer)).rgba;
	textureData = mix(s1, s2, 0.5);
#else
	textureData = texture(texture3, vec3(uv, layer)).rgba;
#endif
}

void SampleTerrainTexture(out vec4 diffuseColor)
{
#ifdef TEXTURE0
	// LookupTexture & slope/height texture coords

	// size of the blending tap in fractions of a pixel
	#define TAP_EPSILON 0.45
	#define ONE_MINUS_TAP_EPSILON (1.0 - TAP_EPSILON)
	vec2 texSize = textureSize(texture2, 0).rg;

	// get the location of the current UV coordinate inside the sampled texel
	vec2 texelCoord = mod(uv1 * texSize, 1.0);

	// get the offset of the coordinate relative to the texel center and make offsets
	vec2 tapOffset = sign(texelCoord - vec2(0.5)) * vec2(TAP_EPSILON) / texSize;

	// and generate the UVs for the two texture taps.
	vec2 tap1Uv = uv1 + vec2(tapOffset.x, 0.0);
	vec2 tap2Uv = uv1 + vec2(0.0, tapOffset.y);

	// Do a simple multi-tap sampling to blend textures at borders
	float terrainType = texture(texture2, uv1).r;
	float terrainTypeTap1 = texture(texture2, tap1Uv).r;
	float terrainTypeTap2 = texture(texture2, tap2Uv).r;

	// We tap the terrain type lookup three times, but only perform two texture samples against the actual texture
	vec2 absOffset = abs(texelCoord - vec2(0.5)) * 2.0;
	float terrainType2 = 0.0;
	float blendSrc = 0.0;

	// Select the highest-weighted terrain type
	if (absOffset.x > absOffset.y) {
		terrainType2 = terrainTypeTap1;
		blendSrc = absOffset.x;
	} else {
		terrainType2 = terrainTypeTap2;
		blendSrc = absOffset.y;
	}
	float blendWeight = max((blendSrc - ONE_MINUS_TAP_EPSILON) * (1.0 / TAP_EPSILON), 0.0);

	// use the size of the terrain texture array to scale the texture type value
	// FIXME: define this in a uniform and hook it into terrain atlas definition
	float size = textureSize(texture3, 0).z;

	vec4 terrainColor1;
	vec4 terrainColor2;
	ReadTerrainTextureMix(uv0, terrainType * size, terrainColor1);
	ReadTerrainTextureMix(uv0, terrainType2 * size, terrainColor2);

	// lerp between the two textures (blendweight is divided by 2 because the blend is split across two LUT texels)
	diffuseColor = mix(terrainColor1, terrainColor2, blendWeight * 0.5);
#else
	diffuseColor = vec4(0.313, 0.313, 0.313, 1.0);
#endif
}

void main(void)
{
	vec4 hidetail = texture(texture0, uv0 * detailScaleHi);
	vec4 lodetail = texture(texture1, uv0 * detailScaleLo);

    vec4 textureColour;
    SampleTerrainTexture(textureColour);

	vec3 eyeposScaled = varyingEyepos * geosphereInvRadius;
	vec3 eyenorm = normalize(varyingEyepos);
	vec3 tnorm = normalize(varyingNormal);
	vec4 diff = vec4(0.0);

	float surfaceDist = dist * geosphereInvRadius;

	// calculate the detail texture contribution from hi and lo textures
	float hiloMix = exp(-0.004 * dist);
	float detailMix = exp(-0.001 * dist);
	vec4 detailVal = mix(lodetail, hidetail, hiloMix);
	vec4 detailMul = mix(vec4(1.0), detailVal, detailMix);

#ifdef TERRAIN_WITH_WATER
	float specularReflection=0.0;
#endif

#if (NUM_LIGHTS > 0)
	vec3 V = normalize(eyeposScaled - geosphereCenter);

	for (int i=0; i<NUM_LIGHTS; ++i) {
		vec3 L = normalize(uLight[i].position.xyz);
		float uneclipsed = clamp(calcUneclipsed(eclipse, NumShadows, V, L), 0.0, 1.0);
		CalcPlanetDiffuse(diff, uLight[i].diffuse, L, tnorm, uneclipsed);

#ifdef TERRAIN_WITH_WATER
		//water only for specular
	    if (textureColour.b > 0.05 && textureColour.r < 0.05) {
			CalcPlanetSpec(specularReflection, uLight[i], L, tnorm, eyenorm, WATER_SHINE);
		}
#endif
	}

	// Use the detail value to multiply the final colour before lighting
	vec4 ambient = scene.ambient * textureColour;
	vec4 final = textureColour * detailMul * diff;

#ifdef ATMOSPHERE
	float ldprod=0.0;
	float fogFactor=0.0;
	CalcPlanetFogFactor(ldprod, fogFactor, eyenorm, eyeposScaled - geosphereCenter, surfaceDist);

	//calculate sunset tone red when passing through more atmosphere, clamp everything.
	float atmpower = (diff.r+diff.g+diff.b)/3.0;
	vec4 sunset = vec4(0.8,clamp(pow(atmpower,0.8),0.0,1.0),clamp(pow(atmpower,1.2),0.0,1.0),1.0);

	final = fogFactor * (ambient + final);
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
	frag_color +=
		(1.0-fogFactor) * (diff*atmosColor) +
#ifdef TERRAIN_WITH_WATER
		  diff * specularReflection * sunset +
#endif
		  (0.02-clamp(fogFactor,0.0,0.01))*diff*ldprod*sunset +	      //increase fog scatter
		  (pow((1.0-pow(fogFactor,0.75)),256.0)*0.4*diff*atmosColor)*sunset;  //distant fog.
#endif //ATMOSPHERE

#else // NUM_LIGHTS > 0 -- unlit rendering - stars
	//emission is used to boost colour of stars, which is a bit odd
	frag_color = material.emission + textureColour;
#endif
}
