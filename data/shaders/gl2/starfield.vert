// Copyright © 2008-2014 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

varying vec4 color;

uniform Material material;

void main(void)
{
	gl_Position = gl_ModelViewProjectionMatrix * gl_Vertex;
	gl_PointSize = 1.0 + pow(gl_Color.r,3.0);
	color = gl_Color * material.emission;
}
