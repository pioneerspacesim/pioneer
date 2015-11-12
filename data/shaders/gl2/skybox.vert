// Copyright © 2008-2015 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#include "attributes.glsl"
#include "logz.glsl"
#include "lib.glsl"

uniform vec4 u_viewPosition;
uniform Material material;

varying vec3 v_texCoord;
varying float v_skyboxFactor;

void main( void )
{
    vec3 position = a_vertex.xyz;
    position += u_viewPosition.xyz;
    gl_Position = uViewProjectionMatrix * vec4(position, 1.0);
    v_texCoord    = a_vertex.xyz;    
	v_skyboxFactor = material.shininess;
}
