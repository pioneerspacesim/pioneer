// Copyright © 2008-2021 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#include "attributes.glsl"
#include "lib.glsl"

uniform sampler2D texture0; //diffuse
in vec2 texCoord0;

in vec4 vertexColor;

out vec4 frag_color;

void main(void)
{
	frag_color = vertexColor * texture(texture0, texCoord0).x;
}
