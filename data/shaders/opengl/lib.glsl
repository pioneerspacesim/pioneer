// Copyright © 2008-2024 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#include "sRGB.glsl"

// Simple ray-sphere intersection test, assuming ray starts at origin and rayDir is pre-normalized.
// Returns distance to first and second intersections in {x, y} or 0.0 if no intersection.
vec2 raySphereIntersect(in vec3 sphereCenter, in vec3 rayDir, in float radius)
{
	vec3 v = -sphereCenter;
	float b = -dot(v, rayDir);
	float det = (b * b) - dot(v, v) + (radius * radius);
	float sdet = sqrt(det);

	return det > 0.0 ? max(vec2(b - sdet, b + sdet), vec2(0.0)) : vec2(0.0);
}

// ray starts at origin, rayDir and axis are pre-normalized
// Returns distance to first and second intersections in {x, y} or 0.0 if no intersection.
vec2 rayCylinderIntersect(in vec3 rayDir, in vec3 cylinderCenter, in vec3 axis, in float radius)
{
    // tangent vectors (parallel to axis)
    vec3 tray = axis * dot(rayDir, axis);
    vec3 tcenter = axis * dot(cylinderCenter, axis);

    // normal vectors (perpendicular to axis)
    vec3 nray = rayDir - tray;
    vec3 ncenter = cylinderCenter - tcenter;

    // coefficient to move from projection to actual 3d space
    // e.g. if angle between axis and tray = 30deg, actual intersect should be doubled
    float scale = length(nray);

    // intersection given in main plane projection
    vec2 intersect = raySphereIntersect(ncenter, normalize(nray), radius);

    return (scale == 0.f) ? vec2(0.f) : intersect / scale;
}

// Phase functions
// https://www.scratchapixel.com/lessons/procedural-generation-virtual-worlds/simulating-sky/simulating-colors-of-the-sky.html
float miePhaseFunction(const float g, const float mu)
{
	/*
	 * Mie phase function:
	 */
	return 3.f / (8.f * 3.141592) * ((1.f - g * g) * (1.f + mu * mu)) / ((2.f + g * g) * pow(1.f + g * g - 2.f * g * mu, 1.5f));
}

float rayleighPhaseFunction(const float mu)
{
	/*
	 * Rayleigh phase function:
	 */
	return 3.f / (16.f * 3.141592) * (1 + mu * mu);
}

#ifdef FRAGMENT_SHADER

struct Surface {
	vec4 color;
	vec3 specular;
	float shininess;
	vec3 normal;
	vec3 emissive;
	float ambientOcclusion;
};


// Currently used by: hopefully everything
// Evaluates a standard blinn-phong diffuse+specular, with the addition of a
// light intensity term to scale the lighting contribution based on (pre-calculated)
// global occlusion
void BlinnPhongDirectionalLight(in Light light, in float intensity, in Surface surf, in vec3 fragPos, inout vec3 diffuse, inout vec3 specular)
{
	// This code calculates directional lights
	vec3 L = normalize(light.position.xyz); // surface->light vector
	vec3 V = normalize(-fragPos); // surface->eye vector
	vec3 H = normalize(L + V); // halfway vector
	diffuse += surf.color.xyz * light.diffuse.xyz * intensity * max(dot(L, surf.normal), 0.0);
	specular += surf.specular * light.specular.xyz * intensity * pow(max(dot(H, surf.normal), 0.0), surf.shininess);
}

// Used by: geosphere shaders
// Calculate length*density product of a line through the atmosphere
// a - start coord (normalized relative to atmosphere radius)
// b - end coord " "
// centerDensity - atmospheric density at centre of sphere
// length - real length of line in meters
float AtmosLengthDensityProduct(vec3 a, vec3 b, float surfaceDensity, float len, float invScaleHeight)
{
	/* 4 samples */
	float ldprod = 0.0;

	vec3 dir = b-a;
	float ln = max(length(b)-1.0, 0.0);

	/* simple 6-tap raymarch through the atmosphere to sample an average density */
	ldprod = surfaceDensity * (
			exp(-invScaleHeight * (length(a)           -1.0)) +
			exp(-invScaleHeight * (length(a + 0.2*dir) -1.0)) +
			exp(-invScaleHeight * (length(a + 0.4*dir) -1.0)) +
			exp(-invScaleHeight * (length(a + 0.6*dir) -1.0)) +
			exp(-invScaleHeight * (length(a + 0.8*dir) -1.0)) +
			exp(-invScaleHeight * (ln)));

	ldprod *= len;
	return ldprod;
}

#endif
