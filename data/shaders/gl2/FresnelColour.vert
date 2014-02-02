// Copyright © 2008-2014 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

varying vec3 varyingEyepos;
varying vec3 varyingNormal;

void main(void)
{
	gl_Position = logarithmicTransform();

	varyingEyepos = vec3(gl_ModelViewMatrix * gl_Vertex);
	varyingNormal = normalize(gl_NormalMatrix * gl_Normal);
}
