// Copyright © 2008-2014 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

out vec4 vertexColor;

void main(void)
{
	gl_Position = logarithmicTransform();
	vertexColor = a_color;
}
