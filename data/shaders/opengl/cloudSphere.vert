// Copyright © 2008-2016 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#include "attributes.glsl"
#include "logz.glsl"
#include "lib.glsl"
#include "noise.glsl"

out vec3 varyingEyepos;
out vec3 varyingNormal;
out vec3 v_texCoord3D;

uniform vec3 geosphereCenter;
uniform float geosphereRadius;

void main(void)
{
	gl_Position = logarithmicTransform();
	varyingEyepos = vec3(uViewMatrix * a_vertex);
	varyingNormal = vec3(uNormalMatrix * a_normal);
	
	// Animate rotating the clouds around the planet
	float c = cos(time);
	float s = sin(time);
	mat3 m = mat3(	vec3(c, 0, -s), 
					vec3(0, 1, 0),
					vec3(s, 0, c) );
			
	v_texCoord3D = m * a_normal.xyz;
}
