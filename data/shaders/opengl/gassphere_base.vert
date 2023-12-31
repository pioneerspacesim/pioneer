// Copyright © 2008-2024 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#include "attributes.glsl"
#include "lib.glsl"
#include "basesphere_uniforms.glsl"

out vec3 varyingEyepos;
out vec3 varyingNormal;
out vec3 varyingTexCoord0;

void main(void)
{
	gl_Position = matrixTransform();
	varyingEyepos = vec3(uViewMatrix * a_vertex);
	varyingNormal = normalize(normalMatrix() * a_normal);
	varyingTexCoord0 = a_normal.xyz;
}
