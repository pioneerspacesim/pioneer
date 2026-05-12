// Copyright © 2008-2026 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#include "attributes.glsl"
#include "lib.glsl"

in vec2 v_uv;
in vec4 v_color;

out vec4 frag_color;

void main(void)
{
	vec2 uv = v_uv * 2.0 - 1.0;
	float r = length(uv);
	if (r > 1.0) discard;

	float edge = smoothstep(1.0, 0.12, r);

	float alpha = v_color.a * edge;
	frag_color = vec4(v_color.rgb, alpha);
}
