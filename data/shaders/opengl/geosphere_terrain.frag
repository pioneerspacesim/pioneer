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
uniform sampler2D scatterLUT;

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

#if (NUM_LIGHTS > 0)
	vec3 V = normalize(eyeposScaled - geosphereCenter);
	vec3 I = normalize(eyeposScaled - planetIntersect);

	float AU = 149598000000.0;
	frag_color = vec4(0.f);

	// coordinates, in planet radius
	vec4 planet = vec4(geosphereCenter, geosphereRadius);
	vec4 atmosphere = vec4(geosphereCenter, geosphereAtmosTopRad);

	vec4 terrainColor = vec4(0.f);
	vec4 atmosphereColor = vec4(0.f);

	for (int i=0; i<NUM_LIGHTS; ++i) {
		vec3 terrainDiffIn = vec3(0.f);
		vec3 terrainDiffOut = vec3(0.f);
		vec3 atmosphereDiff = vec3(0.f);
		vec3 waterSpecular = vec3(0.f);

		vec3 L = normalize(uLight[i].position.xyz);
		float uneclipsed = clamp(calcUneclipsed(eclipse, NumShadows, V, L), 0.0, 1.0);
		CalcPlanetDiffuse(diff, uLight[i].diffuse, L, tnorm, uneclipsed);

		vec3 lightPosAU = uLight[i].position.xyz / AU;
		float intensity = 1.f / dot(lightPosAU, lightPosAU); // magic to avoid calculating length and then squaring it

		vec4 lightColor = toLinear(uLight[i].diffuse);
		lightColor.xyz = vec3(1.f, 1.f, 1.f);

		// Color loss through atmosphere from sun to terrain
		terrainDiffIn = calculateTerrainColor(planet, atmosphere, lightColor, L, (I - geosphereCenter) * geosphereRadius, eyenorm, uneclipsed, scatterLUT);
#ifdef TERRAIN_WITH_WATER
		//water only for specular
		if (vertexColor.b > 0.05 && vertexColor.r < 0.05) {
			waterSpecular = calculateAtmosphereColor(planet, atmosphere, lightColor, reflect(L, I), (I - geosphereCenter) * geosphereRadius, eyenorm, uneclipsed, scatterLUT);

			terrainDiffIn = vec3(0.f);
		}
#endif
		// start with diffuse terrain color
		vec3 terrain = vertexColor.xyz;

		terrain *= max(0.f, dot(L, I));

		terrain *= terrainDiffIn;

		// add water reflections
		terrain += waterSpecular * 20;

		terrainDiffOut = calculateTerrainColor(planet, atmosphere, lightColor, L, (I - geosphereCenter) * geosphereRadius, -eyenorm, uneclipsed, scatterLUT);

		// some light is again lost in atmosphere
		terrain *= terrainDiffOut;

		atmosphereDiff = calculateAtmosphereColor(planet, atmosphere, lightColor, L, vec3(0.0), eyenorm, uneclipsed, scatterLUT);

		terrain += atmosphereDiff * 20;

		terrain *= intensity;

		terrainColor.xyz += terrain;
	}

	// Use the detail value to multiply the final colour before lighting
	vec4 ambient = scene.ambient * vertexColor * 0.f;
	vec4 final = detailMul * terrainColor;

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

#else // NUM_LIGHTS > 0 -- unlit rendering - stars
	//emission is used to boost colour of stars, which is a bit odd
	frag_color = material.emission + vertexColor;
#endif

	frag_color = toSRGB(1 - exp(-frag_color));
}
