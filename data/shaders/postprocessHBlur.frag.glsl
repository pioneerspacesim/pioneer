uniform sampler2D fboTex;

void main(void)
{
	const float inc = 1.0/128.0;
	float div;
	vec4 col = texture2D(fboTex, vec2(gl_TexCoord[0].s, gl_TexCoord[0].t))*0.5;
	col += texture2D(fboTex, vec2(gl_TexCoord[0].s+4.0*inc, gl_TexCoord[0].t)) * 0.00390625;
	col += texture2D(fboTex, vec2(gl_TexCoord[0].s+3.0*inc, gl_TexCoord[0].t)) * 0.015625;
	col += texture2D(fboTex, vec2(gl_TexCoord[0].s+2.0*inc, gl_TexCoord[0].t)) * 0.0625;
	col += texture2D(fboTex, vec2(gl_TexCoord[0].s+1.0*inc, gl_TexCoord[0].t)) * 0.25;
	col += texture2D(fboTex, vec2(gl_TexCoord[0].s-4.0*inc, gl_TexCoord[0].t+4.0*inc)) * 0.00390625;
	col += texture2D(fboTex, vec2(gl_TexCoord[0].s-3.0*inc, gl_TexCoord[0].t+3.0*inc)) * 0.015625;
	col += texture2D(fboTex, vec2(gl_TexCoord[0].s-2.0*inc, gl_TexCoord[0].t+2.0*inc)) * 0.0625;
	col += texture2D(fboTex, vec2(gl_TexCoord[0].s-1.0*inc, gl_TexCoord[0].t+1.0*inc)) * 0.25;
	if ((col.r > col.g) && (col.r > col.b)) div = col.r;
	else if (col.g > col.b) div = col.g;
	else div = col.b;
	div = div / (div + 1.0);
//	col = col * div;
	col = col / (col + vec4(1.0));
	gl_FragColor = col;
}
