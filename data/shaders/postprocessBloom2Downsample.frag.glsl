uniform sampler2DRect fboTex;
// 2nd downscale stage of making the bloom texture
// simple downscale

void main(void)
{
	vec2 p = 2.0*gl_FragCoord.xy;
	vec3 col = vec3(texture2DRect(fboTex, vec2(p.x, p.y)));
	col += vec3(texture2DRect(fboTex, vec2(p.x+1.0, p.y)));
	col += vec3(texture2DRect(fboTex, vec2(p.x+1.0, p.y+1.0)));
	col += vec3(texture2DRect(fboTex, vec2(p.x, p.y+1.0)));
	col *= 0.25;
	gl_FragColor = vec4(col.r, col.g, col.b, 0.0);
}
