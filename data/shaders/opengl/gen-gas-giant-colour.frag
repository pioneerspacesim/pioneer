// Copyright © 2008-2015 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#include "attributes.glsl"
#include "logz.glsl"
#include "lib.glsl"
#include "noise.glsl"

uniform sampler2D texture2; // ???

uniform vec3 v0;
uniform vec3 v1;
uniform vec3 v2;
uniform vec3 v3;
uniform float fracStep;

uniform vec3 frequency;

in vec3 vertex;
in vec2 uv;

out vec4 frag_color;

#ifdef GEN_JUPITER_ESQUE
vec4 GetColour(in vec3 p)
{	
	float n1 = fbm(p * 4.0, 8, frequency[0], 0.5);
	float n2 = fbm(p * 3.14159, 8, frequency[2], 0.5);
	vec4 color = vec4(texture(texture2, vec2(0.0, (p.y + 1.0) * 0.5) + vec2(n1*0.075,n2*0.075)).xyz, 1.0);
	return color;
}
#endif

#ifdef GEN_SATURN_ESQUE
vec4 GetColour(in vec3 p)
{
	float n1 = fbm(p * 4.0, 8, frequency[0], 0.5);
	float n2 = fbm(p * 3.14159, 8, frequency[2], 0.5);
	vec4 color = vec4(texture(texture2, vec2(0.0, (p.y + 1.0) * 0.5) + vec2(n1*0.075,n2*0.075)).xyz, 1.0);
	return color;
}
#endif

#ifdef GEN_SATURN2_ESQUE
vec4 GetColour(in vec3 p)
{
	float n1 = fbm(p * 4.0, 8, frequency[0], 0.5);
	float n2 = fbm(p * 3.14159, 8, frequency[2], 0.5);
	vec4 color = vec4(texture(texture2, vec2(0.0, (p.y + 1.0) * 0.5) + vec2(n1*0.075,n2*0.075)).xyz, 1.0);
	return color;
}
#endif // GEN_SATURN2_ESQUE

#ifdef GEN_NEPTUNE_ESQUE
vec4 GetColour(in vec3 p)
{
	float n1 = fbm(p * 4.0, 8, frequency[0], 0.5);
	float n2 = fbm(p * 3.14159, 8, frequency[2], 0.5);
	vec4 color = vec4(texture(texture2, vec2(0.0, (p.y + 1.0) * 0.5) + vec2(n1*0.075,n2*0.075)).xyz, 1.0);
	return color;
}
#endif

#ifdef GEN_NEPTUNE2_ESQUE
vec4 GetColour(in vec3 p)
{
	float n1 = fbm(p * 4.0, 8, frequency[0], 0.5);
	float n2 = fbm(p * 3.14159, 8, frequency[2], 0.5);
	vec4 color = vec4(texture(texture2, vec2(0.0, (p.y + 1.0) * 0.5) + vec2(n1*0.075,n2*0.075)).xyz, 1.0);
	return color;
}
#endif

#ifdef GEN_URANUS_ESQUE 
vec4 GetColour(in vec3 p)
{
	float n1 = fbm(p * 4.0, 8, frequency[0], 0.5);
	float n2 = fbm(p * 3.14159, 8, frequency[2], 0.5);
	vec4 color = vec4(texture(texture2, vec2(0.0, (p.y + 1.0) * 0.5) + vec2(n1*0.075,n2*0.075)).xyz, 1.0);
	return color;
}
#endif

// in patch surface coords, [0,1]
// v[0] to v[3] are the corner vertices
vec3 GetSpherePoint(in float x, in float y) {
	return normalize(v0 + x*(1.0-y)*(v1-v0) + x*y*(v2-v0) + (1.0-x)*y*(v3-v0));
}

void main(void)
{
	float xfrac = (uv.x-0.5) * fracStep;
	float yfrac = (uv.y-0.5) * fracStep;
	vec3 p = GetSpherePoint(xfrac, yfrac);
	
	// call the GetColour function implemented for this shader type
	vec4 colour = GetColour(p);
	
	frag_color = colour;
	
	SetFragDepth();
}
