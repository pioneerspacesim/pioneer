// Copyright © 2008-2014 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

void main(void)
{
	gl_Position = logarithmicTransform();

	gl_TexCoord[0] = gl_MultiTexCoord0;
	gl_TexCoord[1] = gl_Vertex;

	gl_FrontColor = gl_Color;
}
