uniform vec4 color;
#ifdef TEXTURE0
uniform sampler2D texture0;
varying vec2 texCoord0;
#endif

void main(void)
{
#ifdef TEXTURE0
	gl_FragColor = texture2D(texture0, texCoord0) * color;
#else
	gl_FragColor = color;
#endif
	SetFragDepth(gl_TexCoord[6].z);
}
