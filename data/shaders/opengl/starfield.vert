// Copyright © 2008-2024 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#include "attributes.glsl"
#include "lib.glsl"

out vec4 v_color;

void main(void)
{
	gl_Position = uViewProjectionMatrix * a_vertex;
	gl_PointSize = 1.0 + pow(a_normal.z,3.0);
	v_color = a_color;
}
