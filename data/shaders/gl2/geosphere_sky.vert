varying vec4 varyingEyepos;
varying vec3 varyingNormal;

void main(void)
{
	gl_Position = logarithmicTransform();
	varyingEyepos = gl_ModelViewMatrix * gl_Vertex;
	varyingNormal = gl_NormalMatrix * gl_Normal;
}

