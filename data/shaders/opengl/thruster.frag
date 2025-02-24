// Copyright © 2008-2025 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#include "attributes.glsl"
#include "lib.glsl"

uniform sampler2D texture0; //diffuse

in vec2 texCoord0;
in vec4 flame_color;
in float ramp;

out vec4 frag_color;

void main(void)
{
	vec4 color = texture(texture0, texCoord0);
	vec3 old_color = color.rgb;
	color *= flame_color;
	// generate a light blue flickering glow around the flame
	float ramp2 = ramp * ramp;
	color.g += ramp2 * ramp2 * old_color.g;
	color.r += ramp2 * ramp * old_color.r;
	color.b += ramp2 * old_color.b;

	frag_color = color;
}
