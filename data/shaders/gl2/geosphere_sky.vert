// Copyright © 2008-2014 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

varying vec4 varyingEyepos;

void main(void)
{
	gl_Position = logarithmicTransform();
	varyingEyepos = gl_ModelViewMatrix * gl_Vertex;
}

