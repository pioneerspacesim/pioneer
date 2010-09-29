uniform sampler2DRect fboTex;
// downscale stage of making the bloom texture

const float incX = (800.0/128.0)/4.0;
const float incY = (600.0/128.0)/4.0;

void main(void)
{
	vec4 col = vec4(0.0);
	col += texture2DRect(fboTex, vec2(gl_TexCoord[0].s, gl_TexCoord[0].t));
	col += texture2DRect(fboTex, vec2(gl_TexCoord[0].s+1.0*incX, gl_TexCoord[0].t));
	col += texture2DRect(fboTex, vec2(gl_TexCoord[0].s+2.0*incX, gl_TexCoord[0].t));
	col += texture2DRect(fboTex, vec2(gl_TexCoord[0].s+3.0*incX, gl_TexCoord[0].t));
	col += texture2DRect(fboTex, vec2(gl_TexCoord[0].s, gl_TexCoord[0].t+1.0*incY));
	col += texture2DRect(fboTex, vec2(gl_TexCoord[0].s+1.0*incX, gl_TexCoord[0].t+1.0*incY));
	col += texture2DRect(fboTex, vec2(gl_TexCoord[0].s+2.0*incX, gl_TexCoord[0].t+1.0*incY));
	col += texture2DRect(fboTex, vec2(gl_TexCoord[0].s+3.0*incX, gl_TexCoord[0].t+1.0*incY));
	col += texture2DRect(fboTex, vec2(gl_TexCoord[0].s, gl_TexCoord[0].t+2.0*incY));
	col += texture2DRect(fboTex, vec2(gl_TexCoord[0].s+1.0*incX, gl_TexCoord[0].t+2.0*incY));
	col += texture2DRect(fboTex, vec2(gl_TexCoord[0].s+2.0*incX, gl_TexCoord[0].t+2.0*incY));
	col += texture2DRect(fboTex, vec2(gl_TexCoord[0].s+3.0*incX, gl_TexCoord[0].t+2.0*incY));
	col += texture2DRect(fboTex, vec2(gl_TexCoord[0].s, gl_TexCoord[0].t+3.0*incY));
	col += texture2DRect(fboTex, vec2(gl_TexCoord[0].s+1.0*incX, gl_TexCoord[0].t+3.0*incY));
	col += texture2DRect(fboTex, vec2(gl_TexCoord[0].s+2.0*incX, gl_TexCoord[0].t+3.0*incY));
	col += texture2DRect(fboTex, vec2(gl_TexCoord[0].s+3.0*incX, gl_TexCoord[0].t+3.0*incY));
	col /= 16.0;

	col -= vec4(1.0);
	if (col.r < 0.0) col.r = 0.0;
	if (col.g < 0.0) col.g = 0.0;
	if (col.b < 0.0) col.b = 0.0;
	col.a = 1.0;
	gl_FragColor = col;

/*	if ((col.r > 1.0) || (col.g > 1.0) || (col.b > 1.0)) {
		gl_FragColor = col;
	} else {
		gl_FragColor = vec4(0.0);
	}*/
}
