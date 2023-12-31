// Copyright © 2008-2024 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#include "attributes.glsl"
#include "lib.glsl"

out vec4 color;
out vec2 uv;
out vec3 lightDir;

void main(void)
{
	gl_Position = matrixTransform();
	color = a_color;
	uv = a_uv0.xy * 2.0 - 1.0; //recenter to -1,1 range
	lightDir = normalize(-a_vertex.xyz);
}
