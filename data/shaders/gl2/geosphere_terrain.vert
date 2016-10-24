// Copyright © 2008-2015 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#include "attributes.glsl"
#include "logz.glsl"
#include "lib.glsl"

varying vec3 varyingEyepos;
varying vec3 varyingNormal;
varying vec4 vertexColor;

uniform vec3 geosphereCenter;
uniform float geosphereRadius;

#ifdef DETAIL_MAPS
varying vec2 texCoord0;
varying float dist;
#endif // DETAIL_MAPS

#ifdef TERRAIN_WITH_LAVA
varying vec4 varyingEmission;
uniform Material material;
#endif

void main(void)
{
	gl_Position = logarithmicTransform();
	vertexColor = a_color;
	varyingEyepos = vec3(uViewMatrix * a_vertex);
	varyingNormal = normalize(uNormalMatrix * a_normal);
	
#ifdef DETAIL_MAPS
	texCoord0 = a_uv0.xy;
	dist = abs(varyingEyepos.z);
#endif // DETAIL_MAPS

#ifdef TERRAIN_WITH_LAVA
	varyingEmission = material.emission;
	//Glow lava terrains
	if ( vertexColor.r > 0.4 && vertexColor.g < 0.2 && vertexColor.b < 0.4 ) 
	{
		varyingEmission = 3.0*vertexColor;
		varyingEmission *= (vertexColor.r+vertexColor.g+vertexColor.b);
	}
#endif
}
