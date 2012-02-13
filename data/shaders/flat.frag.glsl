#ifdef TEXTURE0
varying vec2 texCoord0;
uniform sampler2D texture0;
#endif
uniform vec4 color;

void main(void)
{
#ifdef TEXTURE0
	gl_FragColor = texture2D(texture0, texCoord0) * color * color.a;
#else
	gl_FragColor = color * color.a;
#endif
	SetFragDepth(gl_TexCoord[6].z);
}
