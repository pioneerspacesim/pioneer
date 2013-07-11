varying vec3 varyingEyepos;
varying vec3 varyingNormal;

void main(void)
{
	gl_Position = logarithmicTransform();

	varyingEyepos = vec3(gl_ModelViewMatrix * gl_Vertex);
	varyingNormal = normalize(gl_NormalMatrix * gl_Normal);
}
