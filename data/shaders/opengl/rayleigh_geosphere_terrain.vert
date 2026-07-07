// Copyright © 2008-2026 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#include "attributes.glsl"
#include "lib.glsl"
#include "basesphere_uniforms.glsl"
#include "rayleigh-lib.glsl"

uniform int NumShadows;
uniform sampler2D scatterLUT;
uniform sampler2D rayleighLUT;
uniform sampler2D mieLUT;

out vec3 varyingEyepos;
out vec3 varyingNormal;
out vec4 vertexColor;

out vec2 texCoord0;
out float dist;

void main(void)
{
	gl_Position = matrixTransform();
	vertexColor = a_color;
	varyingEyepos = vec3(uViewMatrix * a_vertex);
	varyingNormal = normalize(normalMatrix() * a_normal);

	texCoord0 = a_uv0.xy;
	dist = length(varyingEyepos);

	vec3 eyeposScaled = varyingEyepos * geosphereInvRadius;
	vec3 eyenorm = normalize(varyingEyepos);
	vec3 tnorm = normalize(varyingNormal);
	vec4 diff = vec4(0.0);

	vec2 planetDist = raySphereIntersect(geosphereCenter, eyenorm, geosphereRadius);
	vec3 planetIntersect = geosphereCenter + (eyenorm * planetDist.x);

#if (NUM_LIGHTS > 0)
	vec3 V = normalize(eyeposScaled - geosphereCenter);
	vec3 I = normalize(eyeposScaled - planetIntersect);

	float AU = 149598000000.0;

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

		// Color loss through atmosphere from sun to terrain
		terrainDiffIn = calculateTerrainColor(planet, atmosphere, lightColor, L, (I - geosphereCenter) * geosphereRadius, eyenorm, uneclipsed, scatterLUT, rayleighLUT, mieLUT);
#ifdef TERRAIN_WITH_WATER
		//water only for specular
		if (vertexColor.b > 0.05 && vertexColor.r < 0.05) {
			waterSpecular = calculateAtmosphereColor(planet, atmosphere, lightColor, reflect(L, I), (I - geosphereCenter) * geosphereRadius, eyenorm, uneclipsed, scatterLUT, rayleighLUT, mieLUT);

			terrainDiffIn = vec3(0.f);
		}
#endif
		// start with diffuse terrain color
		vec3 terrain = vertexColor.xyz * intensity;
		terrain *= terrainDiffIn;
		terrain *= max(0.f, dot(L, I));

		// add water reflections
		terrain += waterSpecular * intensity * 20;

		// add lava glow
#ifdef TERRAIN_WITH_LAVA
		if ( vertexColor.r > 0.4 && vertexColor.g < 0.2 && vertexColor.b < 0.4 ) {
			terrain += 3.0 * vertexColor.xyz * (vertexColor.r + vertexColor.g + vertexColor.b);
		} else {
			terrain += material.emission.xyz;
		}
#endif

		terrainDiffOut = calculateTerrainColor(planet, atmosphere, lightColor, I, (I - geosphereCenter) * geosphereRadius, eyenorm, uneclipsed, scatterLUT, rayleighLUT, mieLUT);

		// some light is again lost in atmosphere
		terrain *= terrainDiffOut;

		atmosphereDiff = calculateAtmosphereColor(planet, atmosphere, lightColor, L, vec3(0.0), eyenorm, uneclipsed, scatterLUT, rayleighLUT, mieLUT);

		terrain += atmosphereDiff * intensity * 20;

		terrainColor.xyz += terrain;
	}

	vertexColor = terrainColor;
#endif
}
