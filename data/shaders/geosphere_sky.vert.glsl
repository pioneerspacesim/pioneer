
varying vec4 varyingEyepos;

void main(void)
{
#ifdef ZHACK
	gl_Position = logarithmicTransform();
#else
	gl_Position = ftransform();
#endif
	varyingEyepos = gl_ModelViewMatrix * gl_Vertex;
}
