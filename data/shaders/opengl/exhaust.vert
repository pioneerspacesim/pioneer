// Copyright © 2008-2026 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#include "attributes.glsl"
#include "lib.glsl"

out vec2 v_uv;
out float v_ageNorm;
out float v_opacityScale;

void main(void)
{
	gl_Position = matrixTransform();
	v_uv = a_uv0.xy;
	v_ageNorm = a_normal.x;
	v_opacityScale = a_normal.y;
}
