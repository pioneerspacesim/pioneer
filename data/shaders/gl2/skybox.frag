// Copyright © 2008-2015 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#include "attributes.glsl"
#include "logz.glsl"
#include "lib.glsl"

uniform samplerCube texture0;

varying vec3 v_texCoord;
varying float v_skyboxFactor;

void main( void )
{
    gl_FragColor = vec4(textureCube( texture0, v_texCoord ).xyz * v_skyboxFactor, 1.0);
}
