// Copyright © 2008-2023 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#include "eclipse.glsl"

layout(std140) uniform BaseSphereData {
	// to keep distances sane we do a nearer, smaller scam. this is how many times
	// smaller the geosphere has been made
	vec3 geosphereCenter;				// TODO documentation
	float geosphereRadius;				// (planet radius)
	float geosphereInvRadius;			// 1.0 / (planet radius)
	float geosphereAtmosTopRad;			// in planet radii
	float geosphereAtmosFogDensity;		// TODO documentation
	float geosphereAtmosInvScaleHeight; // TODO documentation
	vec4 atmosColor;

	// Eclipse data
	Eclipse eclipse;
};
