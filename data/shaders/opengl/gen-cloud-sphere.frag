// Copyright © 2008-2015 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#include "attributes.glsl"
#include "logz.glsl"
#include "noise.glsl"

uniform vec3 v0;
uniform vec3 v1;
uniform vec3 v2;
uniform vec3 v3;
uniform float fracStep;
uniform float hueAdjust;

#ifndef FBM_OCTAVES
#define FBM_OCTAVES 8
#endif

const float CloudCover = 0.25;
const float CloudSharpness = 0.25;
const float nScale = 1.4; // Uniform?
const float Density = 0.02;
const float ESun = 1.0;

in vec3 vertex;
in vec2 uv;

out vec4 frag_color;

float CloudExpCurve(float v)
{
	float c = max(v - CloudCover,0.0);
	return 1.0 - pow(CloudSharpness, c);
}

vec4 GetColour(in vec3 p)
{
	// generate some noise clouds
	// calculate solar heating + height & water contributions
	float distort = fbm(p, 5, 2, 0.5) * 0.25;
	float intensity = clamp((1.0 - abs(p.y)) + distort, 0.0, 1.0);
	float heatAbsorption = intensity*1.5*ESun;
		
	// 1st cloud set
	float curvenoise = fbm(p, 6, 8.0, 0.5) * 2.0;
	float curve = CloudExpCurve(curvenoise);
	
	// 2nd cloud set
	vec3 noisePosition = p * 10.0;
	float noise = fbm(noisePosition, 6, 8.0, 0.5) * nScale;
	float rnoise = ridgedNoise(noisePosition, 4, 1.0, 0.5) * nScale;
	rnoise -= (1.0 - Density);
	
	// combine
	float thickness = max(rnoise * 2.0 + noise, 0.0);
	thickness = min(heatAbsorption * (((thickness * thickness)) + curve), 1.0);
	vec4 texColor = vec4(vec3(thickness,thickness,thickness)*2.0, 1.0);
	// end of noise clouds
	return texColor;
}

// in patch surface coords, [0,1]
// v[0] to v[3] are the corner vertices
vec3 GetSpherePoint(in float x, in float y) {
	return normalize(v0 + x*(1.0-y)*(v1-v0) + x*y*(v2-v0) + (1.0-x)*y*(v3-v0));
}

void main(void)
{
	float xfrac = (uv.x-0.5) * fracStep;
	float yfrac = (uv.y-0.5) * fracStep;
	
	// call the GetColour function implemented for this shader type
	// Hue Shift the colour and store the final result
	frag_color = GetColour(GetSpherePoint(xfrac, yfrac));
	
	SetFragDepth();
}
