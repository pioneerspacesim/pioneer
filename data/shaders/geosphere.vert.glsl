varying vec3 varyingEyepos;
varying vec3 varyingNormal;

void main(void)
{
	gl_Position = logarithmicTransform();

	gl_FrontColor = gl_Color;
	varyingEyepos = vec3(gl_ModelViewMatrix * gl_Vertex);
	varyingNormal = gl_NormalMatrix * gl_Normal;
}
