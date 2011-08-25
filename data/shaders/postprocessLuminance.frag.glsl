uniform sampler2DRect fboTex;
varying vec2 texcoord;
// downscale stage of making the bloom texture

void main(void)
{
	const float delta = 0.001;
	vec3 col = vec3(texture2DRect(fboTex, texcoord.st));
	gl_FragColor = vec4(log(delta + dot(col, vec3(0.299,0.587,0.114))));
}
