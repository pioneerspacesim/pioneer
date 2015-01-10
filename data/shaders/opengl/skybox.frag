// Copyright © 2008-2015 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

uniform samplerCube texture0;

in vec3 v_texCoord;
in float v_skyboxFactor;

out vec4 frag_color;

void main( void )
{
    frag_color = vec4(texture( texture0, v_texCoord ).xyz * v_skyboxFactor, 1.0);
}
