// Copyright © 2008-2019 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#include "attributes.glsl"
#include "logz.glsl"
#include "lib.glsl"

uniform samplerCube texture0;

in vec3 v_texCoord;
in float v_skyboxFactor;

out vec4 frag_color;

void main( void )
{
    frag_color = vec4(texture( texture0, v_texCoord ).xyz * v_skyboxFactor, 1.0);
}
