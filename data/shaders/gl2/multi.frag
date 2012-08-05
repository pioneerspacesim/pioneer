#ifdef TEXTURE0
uniform sampler2D texture0;
varying vec2 texCoord0;
#endif
#ifdef VERTEXCOLOR
varying vec4 vertexColor;
#endif
void main(void)
{
#ifdef VERTEXCOLOR
	vec4 color = vertexColor;
#else
	vec4 color = vec4(1.0);
#endif
#ifdef TEXTURE0
	color *= texture2D(texture0, texCoord0);
#endif
	gl_FragColor = color;
	SetFragDepth();
}
