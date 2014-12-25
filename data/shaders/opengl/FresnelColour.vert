// Copyright © 2008-2014 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

out vec3 varyingEyepos;
out vec3 varyingNormal;

void main(void)
{
	gl_Position = logarithmicTransform();

	varyingEyepos = vec3(uViewMatrix * a_vertex);
	varyingNormal = vec3(normalize(uNormalMatrix * vec4(a_normal, 0.0)));
}
