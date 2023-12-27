// Copyright © 2008-2024 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#include "attributes.glsl"
#include "lib.glsl"

uniform sampler2D texture0;
in vec2 texCoord0;
in vec4 texCoord1;
in vec4 varyingEyepos;

out vec4 frag_color;

void main(void)
{
	// Bits of ring in shadow!
	vec4 col = vec4(0.0);
	vec4 texCol = texture(texture0, texCoord0);

	vec3 eyenorm = normalize(varyingEyepos.xyz);

	for (int i=0; i<NUM_LIGHTS; ++i) {
		float l = findSphereEyeRayEntryDistance(-vec3(texCoord1), vec3(uViewMatrixInverse * uLight[i].position), 1.0);
		if (l <= 0.0) {
			// first term: diffuse light phase (like in full/new moon)
			float mu = dot(normalize(vec3(uLight[i].position)), eyenorm);
			float diffuse = sqrt((1 - mu) / 2);

			// second term: diffuse mie (scattering through ring)
			float g = 0.76f;
			float phaseThrough = 3.f / (8.f * 3.141592) * ((1.f - g * g) * (1.f + mu * mu)) / ((2.f + g * g) * pow(1.f + g * g - 2.f * g * mu, 1.5f));

			// third term: reflect mie (imitate albedo >= 1.0)
			float muRev = dot(-normalize(vec3(uLight[i].position)), eyenorm);
			float phaseReflect = 3.f / (8.f * 3.141592) * ((1.f - g * g) * (1.f + muRev * muRev)) / ((2.f + g * g) * pow(1.f + g * g - 2.f * g * muRev, 1.5f));

			col = col + texCol * (diffuse + phaseThrough + phaseReflect) * uLight[i].diffuse;
		}
	}
	col.a = texCol.a;
	frag_color = col;
}
