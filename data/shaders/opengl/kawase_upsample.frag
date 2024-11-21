// Copyright © 2008-2024 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

in vec2 v_uv;

uniform sampler2D texture0; //input

out vec4 frag_color;

vec4 upsample(vec2 uv, vec2 halfpixel)
{
	vec2 offset = vec2(halfpixel.x, -halfpixel.y);
	vec2 linearx = vec2(halfpixel.x * 2.0, 0.0);
	vec2 lineary = vec2(0.0, halfpixel.y * 2.0);

	vec4 sum = vec4(0.0);

	sum += texture(texture0, uv - linearx);
	sum += texture(texture0, uv + linearx);
	sum += texture(texture0, uv - lineary);
	sum += texture(texture0, uv + lineary);
	sum += texture(texture0, uv - halfpixel) * 2.0;
	sum += texture(texture0, uv + halfpixel) * 2.0;
	sum += texture(texture0, uv + offset) * 2.0;
	sum += texture(texture0, uv - offset) * 2.0;

	return sum / 12.0;
}

void main(void)
{
	vec2 halfpixel = vec2(1, 1) / vec2(textureSize(texture0, 0));

	frag_color = upsample(v_uv, halfpixel);
}
