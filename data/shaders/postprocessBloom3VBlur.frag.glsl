uniform sampler2DRect fboTex;

void main(void)
{
	vec4 col = texture2DRect(fboTex, vec2(gl_FragCoord.x, gl_FragCoord.y))*0.5;
	col += texture2DRect(fboTex, vec2(gl_FragCoord.x, gl_FragCoord.y-4.0)) * 0.00390625;
	col += texture2DRect(fboTex, vec2(gl_FragCoord.x, gl_FragCoord.y-3.0)) * 0.015625;
	col += texture2DRect(fboTex, vec2(gl_FragCoord.x, gl_FragCoord.y-2.0)) * 0.0625;
	col += texture2DRect(fboTex, vec2(gl_FragCoord.x, gl_FragCoord.y-1.0)) * 0.25;
	col += texture2DRect(fboTex, vec2(gl_FragCoord.x, gl_FragCoord.y+4.0)) * 0.00390625;
	col += texture2DRect(fboTex, vec2(gl_FragCoord.x, gl_FragCoord.y+3.0)) * 0.015625;
	col += texture2DRect(fboTex, vec2(gl_FragCoord.x, gl_FragCoord.y+2.0)) * 0.0625;
	col += texture2DRect(fboTex, vec2(gl_FragCoord.x, gl_FragCoord.y+1.0)) * 0.25;
	gl_FragColor = col * col * 0.0001;
}
