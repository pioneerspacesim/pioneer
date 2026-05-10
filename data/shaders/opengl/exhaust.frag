// Copyright © 2008-2026 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#include "attributes.glsl"
#include "lib.glsl"

in vec2 v_uv;
in vec4 v_color;
in float v_isDust;

out vec4 frag_color;

void main(void)
{
	vec2 uv = v_uv * 2.0 - 1.0;
	float r = length(uv);
	if (r > 1.0) discard;

	float edge = smoothstep(1.0, 0.12, r);
	float inner = smoothstep(0.75, 0.0, r);

	// C++ supplies age fade in v_color: exhaust uses rgb as a uniform scale (ageFade);
	// dust uses rgb as final tint * ageFade. v_color.a holds opacityScale * ageFade.
	vec3 color;
	if (v_isDust > 0.5)
		color = v_color.rgb;
	else
		color = material.diffuse.rgb * v_color.rgb + vec3(0.12, 0.14, 0.18) * inner;

	float alpha = material.diffuse.a * v_color.a * edge;
	frag_color = vec4(color, alpha);
}
