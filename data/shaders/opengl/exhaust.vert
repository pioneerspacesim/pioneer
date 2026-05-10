// Copyright © 2008-2026 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#include "attributes.glsl"
#include "lib.glsl"

out vec2 v_uv;
out vec4 v_color;
// 1.0 when particle is dust kick (no material / inner lift); 0.0 for exhaust plume.
out float v_isDust;

void main(void)
{
	gl_Position = matrixTransform();
	v_uv = a_uv0.xy;
	v_color = vec4(a_color);
	v_isDust = a_normal.x;
}
