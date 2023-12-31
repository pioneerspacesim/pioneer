// Copyright © 2008-2024 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#include "attributes.glsl"

out vec3 v_eyepos;
out vec3 v_normal;

void main(void)
{
	gl_Position = uViewProjectionMatrix * vec4(a_vertex.xyz, 1.0);
	v_eyepos = (uViewMatrix * vec4(a_vertex.xyz, 1.0)).xyz;
	v_normal = normalize(normalMatrix() * a_vertex.xyz);
}
