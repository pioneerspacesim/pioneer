// Copyright © 2008-2015 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

out vec4 color;

uniform Material material;

void main(void)
{
	gl_Position = uViewProjectionMatrix * a_vertex;
	gl_PointSize = 1.0 + pow(a_color.r,3.0);
	color = a_color * material.emission;
}
