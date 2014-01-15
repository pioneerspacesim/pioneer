varying vec3 varyingEyepos;
varying vec3 varyingNormal;
varying vec3 varyingVertex;

void main(void)
{
	gl_Position = logarithmicTransform();

	varyingEyepos = vec3(gl_ModelViewMatrix * gl_Vertex);
	varyingNormal = normalize(gl_NormalMatrix * gl_Normal);
	varyingVertex = gl_Vertex.xyz;
}
