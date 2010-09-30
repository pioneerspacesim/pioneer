uniform sampler2DRect fboTex;
// downscale stage of making the bloom texture

void main(void)
{
	vec4 col = vec4(0.0);
	vec2 p = 2.0*gl_FragCoord.xy;
	col += texture2DRect(fboTex, vec2(p.x, p.y));
	col += texture2DRect(fboTex, vec2(p.x+1.0, p.y));
	col += texture2DRect(fboTex, vec2(p.x+1.0, p.y+1.0));
	col += texture2DRect(fboTex, vec2(p.x, p.y+1.0));
	col *= 0.25;
	if ((col.r > 1.0) || (col.g > 1.0) || (col.b > 1.0)) gl_FragColor = col;
	else gl_FragColor = vec4(0.0);
}
