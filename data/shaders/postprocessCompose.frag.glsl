uniform sampler2DRect fboTex;
uniform sampler2DRect bloomTex;

void main(void)
{
	vec4 col = texture2DRect(fboTex, vec2(gl_FragCoord.x, gl_FragCoord.y));
	col += texture2DRect(bloomTex, vec2(gl_FragCoord.x*0.25, gl_FragCoord.y*0.25));
	col = col / (col + vec4(1.0));
	gl_FragColor = col;
}
