
varying vec4 varyingEyepos;

void main(void)
{
	gl_Position = logarithmicTransform();

	varyingEyepos = gl_ModelViewMatrix * gl_Vertex;
}
