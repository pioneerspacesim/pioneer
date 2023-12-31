// Copyright © 2008-2024 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#include "attributes.glsl"
#include "lib.glsl"

in vec3 varyingEyepos;
in vec3 varyingNormal;
in vec3 varyingVertex;

layout(std140) uniform ShieldData {
	vec4 hits[8];
	float shieldStrength;
	float shieldCooldown;
};

uniform int NumHits;

const vec4 red = vec4(1.0, 0.5, 0.5, 0.5);
const vec4 blue = vec4(0.5, 0.5, 1.0, 1.0);
const vec4 hitColour = vec4(1.0, 0.5, 0.5, 1.0);

out vec4 frag_color;

float calcIntensity(in vec4 hit)
{
	// hit is encoded as { <pos>, life }
	float radius = 50.0 * hit.w;
	vec3 dif = varyingVertex - hit.xyz;
	float sqrDist = dot(dif,dif);

	return clamp(1.0/sqrDist*radius, 0.0, 0.9) * (1.0 - hit.w);
}

void main(void)
{
	vec4 color = mix(red, blue, shieldStrength);
	vec4 fillColour = color * 0.15;

	vec3 eyenorm = normalize(-varyingEyepos);

	float fresnel = 1.0 - abs(dot(eyenorm, varyingNormal)); // Calculate fresnel.
	fresnel = pow(fresnel, 10.0);
	fresnel += 0.05 * (1.0 - fresnel);

	float sumIntensity = 0.0;
	for (int idx=0; idx < NumHits; idx++)
	{
		sumIntensity += calcIntensity(hits[idx]);
	}
	float clampedInt = clamp(sumIntensity, 0.0, 1.0);

	// combine a base colour with the (clamped) fresnel value and fade it out according to the cooldown period.
	color.a = (fillColour.a + clamp(fresnel * 0.5, 0.0, 1.0)) * shieldCooldown;
	// add on our hit effect colour
	color = color + (hitColour * clampedInt);

	frag_color = color;
}
