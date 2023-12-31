// Copyright © 2008-2024 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#include "attributes.glsl"
#include "lib.glsl"

uniform sampler2D texture0;
uniform float coordDownScale;
in vec2 uv;

out vec4 frag_color;

void main(void)
{
#ifdef USE_SPRITE_ATLAS
	frag_color = texture(texture0, (gl_PointCoord * coordDownScale) + uv);
#else
	frag_color = texture(texture0, gl_PointCoord);
#endif
}
