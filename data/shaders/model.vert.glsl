varying vec3 norm;

void main(void)
{
	gl_Position = logarithmicTransform();

	norm = gl_NormalMatrix * gl_Normal;
	gl_TexCoord[0] = gl_MultiTexCoord0;
}
