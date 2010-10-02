uniform sampler2DRect fboTex;
// downscale stage of making the bloom texture

void main(void)
{
	vec3 col = vec3(0.0);
	vec2 p = 2.0*gl_FragCoord.xy;
	vec3 c;
	c = vec3(texture2DRect(fboTex, vec2(p.x, p.y)));
	if (dot(c,c) > 1.0) col += c;
	c = vec3(texture2DRect(fboTex, vec2(p.x+1.0, p.y)));
	if (dot(c,c) > 1.0) col += c;
	c = vec3(texture2DRect(fboTex, vec2(p.x+1.0, p.y+1.0)));
	if (dot(c,c) > 1.0) col += c;
	c = vec3(texture2DRect(fboTex, vec2(p.x, p.y+1.0)));
	if (dot(c,c) > 1.0) col += c;
	col *= 0.25;
	gl_FragColor = vec4(col.r, col.g, col.b, 1.0);
}
