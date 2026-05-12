// Copyright © 2008-2026 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#include "attributes.glsl"
#include "lib.glsl"

out vec2 v_uv;
out vec4 v_color;

void main(void)
{
	gl_Position = matrixTransform();
	v_uv = a_uv0.xy;
	v_color = vec4(a_color);
}
