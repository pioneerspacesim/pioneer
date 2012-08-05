#ifdef TEXTURE0
varying vec2 texCoord0;
#endif

void main(void)
{
	gl_Position = gl_ModelViewProjectionMatrix * gl_Vertex;
#ifdef TEXTURE0
	texCoord0 = gl_MultiTexCoord0.xy;
#endif
}
