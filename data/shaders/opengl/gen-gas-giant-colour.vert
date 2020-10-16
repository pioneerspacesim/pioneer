// Copyright © 2008-2015 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#include "attributes.glsl"

out vec3 vertex;
out vec2 uv;

void main(void)
{
	gl_Position = matrixTransform();
	vertex = (uViewMatrix * a_vertex).xyz;
	uv = a_uv0.xy;
}
