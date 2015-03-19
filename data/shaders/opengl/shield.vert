// Copyright © 2008-2015 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

out vec3 varyingEyepos;
out vec3 varyingNormal;
out vec3 varyingVertex;

void main(void)
{
	gl_Position = logarithmicTransform();

	varyingEyepos = vec3(uViewMatrix * a_vertex);
	varyingNormal = normalize(uNormalMatrix * a_normal);
	varyingVertex = a_vertex.xyz;
}
