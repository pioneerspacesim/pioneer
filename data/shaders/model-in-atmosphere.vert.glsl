// model fragment position in camera space
varying vec4 varyingEyepos;

varying vec3 varyingNormal;

void main(void)
{
#ifdef ZHACK
	gl_Position = logarithmicTransform();
#else
	gl_Position = gl_ModelViewProjectionMatrix * gl_Vertex;
#endif
	gl_FrontColor = gl_Color;
	varyingEyepos = gl_ModelViewMatrix * gl_Vertex;
	varyingNormal = gl_NormalMatrix * gl_Normal;
	gl_TexCoord[0] = gl_MultiTexCoord0;
}
