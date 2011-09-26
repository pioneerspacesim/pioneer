uniform sampler2DRect fboTex;

void main(void)
{
	vec4 col = texture2DRect(fboTex, vec2(gl_FragCoord.x, gl_FragCoord.y))*0.5;
	col += texture2DRect(fboTex, vec2(gl_FragCoord.x-4.0, gl_FragCoord.y)) * 0.00390625;
	col += texture2DRect(fboTex, vec2(gl_FragCoord.x-3.0, gl_FragCoord.y)) * 0.015625;
	col += texture2DRect(fboTex, vec2(gl_FragCoord.x-2.0, gl_FragCoord.y)) * 0.0625;
	col += texture2DRect(fboTex, vec2(gl_FragCoord.x-1.0, gl_FragCoord.y)) * 0.25;
	col += texture2DRect(fboTex, vec2(gl_FragCoord.x+4.0, gl_FragCoord.y)) * 0.00390625;
	col += texture2DRect(fboTex, vec2(gl_FragCoord.x+3.0, gl_FragCoord.y)) * 0.015625;
	col += texture2DRect(fboTex, vec2(gl_FragCoord.x+2.0, gl_FragCoord.y)) * 0.0625;
	col += texture2DRect(fboTex, vec2(gl_FragCoord.x+1.0, gl_FragCoord.y)) * 0.25;
	gl_FragColor = col;
}
