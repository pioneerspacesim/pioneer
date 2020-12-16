// Copyright © 2008-2020 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#include "eclipse.glsl"

layout(std140) uniform GasSphereData {
	// to keep distances sane we do a nearer, smaller scam. this is how many times
	// smaller the geosphere has been made
	vec3 geosphereCenter;
	float geosphereRadius;
	float geosphereInvRadius;
	float geosphereAtmosTopRad;
	float geosphereAtmosFogDensity;
	float geosphereAtmosInvScaleHeight;
	vec4 atmosColor;

	// Eclipse data
	Eclipse eclipse;
};
