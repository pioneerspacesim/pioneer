// Copyright © 2008-2026 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#ifndef TERRAIN_LOCAL_LIGHTS_GLSL
#define TERRAIN_LOCAL_LIGHTS_GLSL

#define MAX_TERRAIN_LOCAL_LIGHTS 16

struct TerrainLocalLight {
	vec4 position; // xyz in unit-sphere coords (planet-frame meters / planet radius)
	vec4 colour;   // rgb, a unused
	vec4 falloff;  // x = range (meters), y = strength, z = additiveFactor, w unused
};

layout(std140) uniform TerrainLocalLightsData {
	int numLights;
	float _pad0;
	vec2 _pad1;
	TerrainLocalLight localLights[MAX_TERRAIN_LOCAL_LIGHTS];
};

#ifdef FRAGMENT_SHADER

uniform vec3 PatchClipCentroid;

void CalcTerrainLocalDiffuse(
	out vec4 multiplyContrib,
	out vec4 additiveContrib,
	in vec3 surfacePosView,
	in vec3 surfaceNormal)
{
	multiplyContrib = vec4(0.0);
	additiveContrib = vec4(0.0);
	if (numLights <= 0)
		return;

	for (int i = 0; i < numLights; ++i) {
		vec3 posUnit = localLights[i].position.xyz;
		vec3 lightViewPos = vec3(uViewMatrix * vec4(posUnit - PatchClipCentroid, 1.0));
		vec3 toLight = lightViewPos - surfacePosView;
		float dist = length(toLight);
		float range = max(localLights[i].falloff.x, 1.0);
		float strength = localLights[i].falloff.y;
		float additiveFactor = clamp(localLights[i].falloff.z, 0.0, 1.0);

		float t = clamp(dist / range, 0.0, 1.0);
		float radial = 1.0 - t;

		vec3 L = toLight / max(dist, 1e-4);
		float ndotl = max(dot(surfaceNormal, L), 0.0);
		const float normalFloor = 0.5;
		float cosine = mix(normalFloor, 1.0, ndotl);

		vec3 lit = localLights[i].colour.rgb * radial * strength * cosine;
		multiplyContrib += vec4(lit * (1.0 - additiveFactor), 0.0);
		additiveContrib += vec4(lit * additiveFactor, 0.0);
	}
}

#endif // FRAGMENT_SHADER

#endif // TERRAIN_LOCAL_LIGHTS_GLSL
