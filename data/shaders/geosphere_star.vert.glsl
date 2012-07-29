
void main(void)
{
	gl_Position = logarithmicTransform();

	gl_FrontColor = gl_Color;
	gl_TexCoord[0] = gl_ModelViewMatrix * gl_Vertex;
	vec3 tnorm = gl_NormalMatrix * gl_Normal;
	gl_TexCoord[1] = vec4(tnorm.x, tnorm.y, tnorm.z, 0.0);
}
