// Copyright © 2008-2026 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#include "attributes.glsl"
#include "lib.glsl"

in vec2 v_uv;
in vec4 v_color;

out vec4 frag_color;

vec2 rando(vec2 st){
    st = vec2( dot(st,vec2(127.1,311.7)),
              dot(st,vec2(269.5,183.3)) );
    return -1.0 + 2.0*fract(sin(st)*43758.5453123);
}

float noise(vec2 st) {
    vec2 i = floor(st);
    vec2 f = fract(st);

    vec2 u = f*f*(3.0-2.0*f);

    return mix( mix( dot( rando(i + vec2(0.0,0.0) ), f - vec2(0.0,0.0) ),
                     dot( rando(i + vec2(1.0,0.0) ), f - vec2(1.0,0.0) ), u.x),
                mix( dot( rando(i + vec2(0.0,1.0) ), f - vec2(0.0,1.0) ),
                     dot( rando(i + vec2(1.0,1.0) ), f - vec2(1.0,1.0) ), u.x), u.y);
}

void main(void)
{
	// Use a circular shape, with a soft edge
	vec2 uv = v_uv * 2.0 - 1.0;
	float r = length(uv);
	if (r > 1.0) discard;
	float edge = smoothstep(1.0, 0.12, r);

	// Make some noise! Woop Woop!
	// Increase noise_cells for smaller blobs, decrease for larger ones.
	const float noise_cells = 3.0;
	vec2 pos = v_uv * noise_cells;
	float noise_val = noise(pos) * 2.0 + 0.5;

	float alpha = v_color.a * edge * noise_val;
	frag_color = vec4(v_color.rgb, alpha);
}
