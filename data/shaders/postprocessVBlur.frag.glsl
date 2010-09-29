uniform sampler2D fboTex;

void main(void)
{
	const float inc = 1.0/128.0;
	vec4 col = texture2D(fboTex, vec2(gl_TexCoord[0].s, gl_TexCoord[0].t))*0.5;
	col += texture2D(fboTex, vec2(gl_TexCoord[0].s, gl_TexCoord[0].t-4.0*inc)) * 0.00390625;
	col += texture2D(fboTex, vec2(gl_TexCoord[0].s, gl_TexCoord[0].t-3.0*inc)) * 0.015625;
	col += texture2D(fboTex, vec2(gl_TexCoord[0].s, gl_TexCoord[0].t-2.0*inc)) * 0.0625;
	col += texture2D(fboTex, vec2(gl_TexCoord[0].s, gl_TexCoord[0].t-1.0*inc)) * 0.25;
	col += texture2D(fboTex, vec2(gl_TexCoord[0].s, gl_TexCoord[0].t+4.0*inc)) * 0.00390625;
	col += texture2D(fboTex, vec2(gl_TexCoord[0].s, gl_TexCoord[0].t+3.0*inc)) * 0.015625;
	col += texture2D(fboTex, vec2(gl_TexCoord[0].s, gl_TexCoord[0].t+2.0*inc)) * 0.0625;
	col += texture2D(fboTex, vec2(gl_TexCoord[0].s, gl_TexCoord[0].t+1.0*inc)) * 0.25;
	gl_FragColor = col;
}
