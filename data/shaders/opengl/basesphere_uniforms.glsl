// Copyright © 2008-2024 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#include "eclipse.glsl"

layout(std140) uniform BaseSphereData {
	// To render accurate planets at any distance or scale, almost all
	// calculations take place in "planet space", with a distance of 1.0
	// defined as equivalent to the planet's nominal radius (planet radii).
	//
	// This allows planets to be rendered in view-space at any scaling factor
	// desired, to play nicely with the depth buffer and any other content
	// rendered (e.g. map views)
	//
	// The atmosphere simulation approximates Rayleigh scattering by
	// calculating the optical density of a path through the atmosphere and
	// assuming a constant in/out scattering factor based on the density
	// approximation.

	vec3 geosphereCenter;				// view-space center of the planet, in planet radii
	float geosphereRadius;				// real planet radius, in meters
	float geosphereInvRadius;			// 1.0 / (view-space radius), converts between view coordinates and planet radii
	float geosphereAtmosTopRad;			// height of the simulated atmosphere, in planet radii
	float geosphereAtmosFogDensity;		// atmospheric density scalar
	float geosphereAtmosInvScaleHeight; // 1.0 / (atmosphere scale height) in planet radii
	vec4 atmosColor;
	vec3 coefficientsR;			// coefficients for approximating the Rayleigh contribution
	vec3 coefficientsM;			// coefficients for approximating the Mie contribution
	vec2 scaleHeight;			// height for (R, M) in km, at which density will be reduced by e

	// Eclipse data
	Eclipse eclipse;
};

// NOTE: you must include attributes.glsl first!

// Common code to calculate the diffuse light term for a planet's surface
// L: light -> surface vector (normalized)
// N: surface normal
// V: surface position relative to the unit-sphere planet
void CalcPlanetDiffuse(inout vec4 diff, in vec4 color, in vec3 L, in vec3 N, in float uneclipsed)
{
	float nDotVP  = max(0.0, dot(N, L));
	float nnDotVP = max(0.0, dot(N, -L));

	//need backlight to increase horizon, attempts to model light propagating towards terminator
	float clampedCosine = (nDotVP + 0.5 * clamp(1.0 - nnDotVP * 4.0, 0, 1) * INV_NUM_LIGHTS);
	diff += color * uneclipsed * 0.5 * clampedCosine;
}

#ifdef FRAGMENT_SHADER

// Common code to calculate the specular light term for a planet's surface
// L: light -> surface vector (normalized)
// N: surface normal
// E: eye->surface vector (normalized)
// power: specular power (blinn-phong)
void CalcPlanetSpec(inout float spec, in Light light, in vec3 L, in vec3 N, in vec3 E, in float power)
{
	//Specular reflection
	vec3 H = normalize(L - E);
	//water only for specular
	spec += pow(max(dot(H, N), 0.0), power) * 0.5 * INV_NUM_LIGHTS;
}

// E: eye->surface direction
// scaledPos: position of the pixel in view space, divided by the radius of the geosphere
void CalcPlanetFogFactor(inout float ldprod, inout float fogFactor, in vec3 E, in vec3 surfacePos, in float dist)
{
	// when does the eye ray intersect atmosphere
	float atmosStart = raySphereIntersect(geosphereCenter, E, geosphereAtmosTopRad).x;
	float atmosDist = (dist - atmosStart) * geosphereRadius;

	// a&b scaled so length of 1.0 means planet surface.
	vec3 a = atmosStart * E - geosphereCenter;
	vec3 b = surfacePos;

	ldprod = AtmosLengthDensityProduct(a, b, atmosColor.w*geosphereAtmosFogDensity, atmosDist, geosphereAtmosInvScaleHeight);
	fogFactor = clamp(1.5 / exp(ldprod), 0.0, 1.0);
}

#endif
