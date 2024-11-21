// Copyright © 2008-2024 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

in vec2 v_uv;

uniform sampler2D texture0; //input

out vec4 frag_color;

vec4 downsample(vec2 uv, vec2 halfpixel)
{
	vec2 offset = vec2(halfpixel.x, -halfpixel.y);

	vec4 sum = texture(texture0, uv) * 4.0;
	sum += texture(texture0, uv - halfpixel);
	sum += texture(texture0, uv + halfpixel);
	sum += texture(texture0, uv + offset);
	sum += texture(texture0, uv - offset);

	return sum / 8.0;
}

void main(void)
{
	vec2 halfpixel = vec2(1, 1) / vec2(textureSize(texture0, 0));

	frag_color = downsample(v_uv, halfpixel);
}
