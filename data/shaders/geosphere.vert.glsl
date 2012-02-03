
varying vec4 varyingEyepos;

void main(void)
{
#ifdef ZHACK
	gl_Position = logarithmicTransform();
#else
	gl_Position = gl_ModelViewProjectionMatrix * gl_Vertex;
#endif
	gl_FrontColor = gl_Color;
	varyingEyepos = gl_ModelViewMatrix * gl_Vertex;
	vec3 tnorm = gl_NormalMatrix * gl_Normal;
	gl_TexCoord[1] = vec4(tnorm.x, tnorm.y, tnorm.z, 0.0);
}
