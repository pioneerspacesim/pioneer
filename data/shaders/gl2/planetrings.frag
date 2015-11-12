// Copyright © 2008-2015 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#include "attributes.glsl"
#include "logz.glsl"
#include "lib.glsl"

uniform sampler2D texture0;
varying vec2 texCoord0;
varying vec4 texCoord1;

void main(void)
{
	// Bits of ring in shadow!
	vec4 col = vec4(0.0);
	vec4 texCol = texture2D(texture0, texCoord0);

	for (int i=0; i<NUM_LIGHTS; ++i) {
		float l = findSphereEyeRayEntryDistance(-vec3(texCoord1), vec3(uViewMatrixInverse * uLight[i].position), 1.0);
		if (l <= 0.0) {
			col = col + texCol*uLight[i].diffuse;
		}
	}
	col.a = texCol.a;
	gl_FragColor = col;

	SetFragDepth();
}
