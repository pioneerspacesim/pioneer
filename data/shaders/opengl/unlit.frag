// Copyright © 2008-2024 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#include "attributes.glsl"
#include "lib.glsl"

#ifdef TEXTURE0
uniform sampler2D texture0; //diffuse
in vec2 texCoord0;
#endif

#ifdef VERTEXCOLOR
in vec4 vertexColor;
#endif

out vec4 frag_color;

void main(void)
{
	vec4 color = material.diffuse;

#ifdef VERTEXCOLOR
	color = vertexColor;
#endif

#ifdef TEXTURE0
	color *= texture(texture0, texCoord0);
#endif

#ifdef ALPHA_TEST
	if (surface.color.a < 0.5)
		discard;
#endif

	frag_color = color;
}
