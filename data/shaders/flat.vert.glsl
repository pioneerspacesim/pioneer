#ifdef TEXTURE0
varying vec2 texCoord0;
#endif

void main(void)
{
	gl_Position = logarithmicTransform();
#ifdef TEXTURE0
	texCoord0 = gl_MultiTexCoord0.xy;
#endif
}

