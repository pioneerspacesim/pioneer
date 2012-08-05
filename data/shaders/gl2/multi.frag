#ifdef TEXTURE0
uniform sampler2D texture0;
varying vec2 texCoord0;
#endif
void main(void)
{
	vec4 color = vec4(1.0, 0.0, 0.2, 1.0);
#ifdef TEXTURE0
	color = texture2D(texture0, texCoord0);
#endif
	gl_FragColor = color;
}
