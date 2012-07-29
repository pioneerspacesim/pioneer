// model fragment position in camera space
varying vec4 varyingEyepos;

varying vec3 varyingNormal;

void main(void)
{
	gl_Position = logarithmicTransform();
	varyingEyepos = gl_ModelViewMatrix * gl_Vertex;
	varyingNormal = gl_NormalMatrix * gl_Normal;
	gl_TexCoord[0] = gl_MultiTexCoord0;
}
