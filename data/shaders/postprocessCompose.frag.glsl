uniform sampler2DRect fboTex;
uniform sampler2D bloomTex;

void main(void)
{
	vec4 col = texture2DRect(fboTex, vec2(gl_FragCoord.x, gl_FragCoord.y));
	col += texture2D(bloomTex, gl_TexCoord[0].st);
	gl_FragColor = col;
}
