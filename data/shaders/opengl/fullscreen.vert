// Copyright © 2008-2024 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#include "attributes.glsl"

out vec2 v_uv;

void main(void)
{
	gl_Position = vec4(a_vertex.xyz, 1.0);
	v_uv = (a_vertex.xy + vec2(1.0)) * vec2(0.5);
}
