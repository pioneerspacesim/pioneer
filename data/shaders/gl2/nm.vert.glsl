varying vec3 ecPos;
varying vec3 norm;
#if TEXTURE0
varying vec2 uv0;
#endif

void main(void)
{
	gl_Position = logarithmicTransform();
	ecPos = vec3(gl_ModelViewMatrix * gl_Vertex);
	norm = normalize(gl_NormalMatrix * gl_Normal);
#if TEXTURE0
	uv0 = gl_MultiTexCoord0.st;
#endif
}

