// Copyright © 2008-2026 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#include "attributes.glsl"
#include "lib.glsl"

in vec2 v_uv;
in float v_ageNorm;
in float v_opacityScale;

out vec4 frag_color;

void main(void)
{
	vec2 uv = v_uv * 2.0 - 1.0;
	float r = length(uv);
	if (r > 1.0) discard;

	float edge = smoothstep(1.0, 0.12, r);
	float inner = smoothstep(0.75, 0.0, r);
	// Nonlinear fade: keep strong near birth, then decay faster through mid/late life.
	float ageFade = pow(1.0 - clamp(v_ageNorm, 0.0, 1.0), 2.8);

	float alpha = material.diffuse.a * edge * ageFade * clamp(v_opacityScale, 0.0, 1.0);
	vec3 color = material.diffuse.rgb + vec3(0.12, 0.14, 0.18) * inner;
	frag_color = vec4(color, alpha);
}
