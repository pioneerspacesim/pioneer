// Copyright © 2008-2025 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#include "attributes.glsl"
#include "lib.glsl"

uniform sampler2D texture0;
uniform sampler2D texture1;
in vec2 texCoord0;
in vec4 texCoord1;
in vec4 varyingEyepos;

out vec4 frag_color;

#define NUM_OCTAVES 3

float findSphereEyeRayEntryDistance(in vec3 sphereCenter, in vec3 eyeTo, in float radius)
{
	return raySphereIntersect(sphereCenter, normalize(eyeTo), radius).x;
}

float octaveNoise(vec2 noiseCoord, float amplitude, float persistence, float lacunarity)
{
	float val = 0.0;
	float frequency = 1.0;

	for (int i = 0; i < NUM_OCTAVES; i++) {
		val += amplitude * texture(texture1, noiseCoord * frequency).r;
		amplitude *= persistence;
		frequency *= lacunarity;
	}

	return val;
}

void main(void)
{
	// Bits of ring in shadow!
	vec4 col = vec4(0.0);
	vec4 texCol = texture(texture0, texCoord0);

	vec3 eyenorm = normalize(varyingEyepos.xyz);

	//noise detail texturing - compute a "density map" for the ring
	float detailDist = length(varyingEyepos);
	vec2 noiseCoord = texCoord1.xz; // using the texCoord1 (a_vertex aka position) probably isn't perfect
	float coarseNoise = 0.3 + 0.7 * clamp(octaveNoise(noiseCoord * 15.0, 0.3, 1.1, 4.0), 0.0, 1.0); // first-level octave noise
	float fineNoise = texture(texture1, noiseCoord * 2000.0).r; //finer detail

	float coarseDistance = remap01(15000000.0, 2500000.0, detailDist);
	float fineDistance = remap01(2000000.0, 200000.0, detailDist);

	//mix between fine detail, coarse detail and white
	float detail1 = mix(1.0, coarseNoise, pow(coarseDistance, 2.0));
	float detail2 = mix(detail1, fineNoise, pow(fineDistance, 2.0));

	float density = (0.2 * detail1 + 0.8 * detail2);

	for (int i=0; i<NUM_LIGHTS; ++i) {
		float l = findSphereEyeRayEntryDistance(-vec3(texCoord1), vec3(uViewMatrixInverse * normalize(uLight[i].position)), 1.0);
		if (l <= 0.0) {
			// first term: diffuse light phase (like in full/new moon)
			float mu = dot(normalize(vec3(uLight[i].position)), eyenorm);
			float diffuse = sqrt((1 - mu) / 2);

			// second term: diffuse mie (scattering through ring)
			float g = 0.76f;
			float phaseThrough = miePhaseFunction(g, mu);

			// third term: reflect mie (imitate albedo >= 1.0)
			float muRev = dot(-normalize(vec3(uLight[i].position)), eyenorm);
			// Reduce impact of retro-reflectance by including density as a second term
			float phaseReflect = density * miePhaseFunction(g, muRev);

			col = col + texCol * (diffuse + phaseThrough + phaseReflect) * uLight[i].diffuse;
		}
	}
	col.a = texCol.a;

	// multiply the colour by the greyscale detail
	frag_color = col * (0.2 + 0.8 * density);
}
