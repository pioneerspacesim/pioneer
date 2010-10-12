uniform sampler2DRect fboTex;
// downscale stage of making the bloom texture

void main(void)
{
	vec3 col = texture2DRect(fboTex, gl_TexCoord[0]);
	gl_FragColor = vec4(dot(col, vec3(0.299,0.587,0.114)));
}
