// Copyright © 2008-2014 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

out vec3 varyingEyepos;
out vec3 varyingNormal;
out vec4 vertexColor;

uniform vec3 geosphereCenter;
uniform float geosphereScaledRadius;

#ifdef TERRAIN_WITH_LAVA
out vec4 varyingEmission;
uniform Material material;
#endif

void main(void)
{
	gl_Position = logarithmicTransform();
	vertexColor = a_color;
	varyingEyepos = vec3(uViewMatrix * a_vertex);
	varyingNormal = vec3(uNormalMatrix * vec4(a_normal, 1.0));

#ifdef TERRAIN_WITH_LAVA
	varyingEmission = material.emission;
	//Glow lava terrains
	if ( vertexColor.r > 0.4 && vertexColor.g < 0.2 && vertexColor.b < 0.4 ) {
		varyingEmission = 3.0*vertexColor;
		varyingEmission *= (vertexColor.r+vertexColor.g+vertexColor.b);

	}
#endif
}
