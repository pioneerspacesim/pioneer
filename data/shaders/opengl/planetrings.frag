// Copyright © 2008-2023 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#include "attributes.glsl"
#include "lib.glsl"

uniform sampler2D texture0;
in vec2 texCoord0;
in vec4 texCoord1;

out vec4 frag_color;

void main(void)
{
	// Bits of ring in shadow!
	vec4 col = vec4(0.0);
	vec4 texCol = texture(texture0, texCoord0);

	for (int i=0; i<NUM_LIGHTS; ++i) {
		float l = findSphereEyeRayEntryDistance(-vec3(texCoord1), vec3(uViewMatrixInverse * uLight[i].position), 1.0);
		if (l <= 0.0) {
			col = col + texCol*uLight[i].diffuse;
		}
	}
	col.a = texCol.a;
	frag_color = col;
}
