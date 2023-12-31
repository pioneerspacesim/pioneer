// Copyright © 2008-2024 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#include "attributes.glsl"

uniform sampler2D texture0; //diffuse

in vec4 v_color;
in vec2 v_texCoord0;

out vec4 frag_color;

void main(void)
{
	frag_color = v_color * texture(texture0, v_texCoord0);
}
