void main(void)
{
#ifdef ZHACK
	gl_Position = logarithmicTransform();
#else
	gl_Position = gl_ModelViewProjectionMatrix * gl_Vertex;
#endif
	gl_TexCoord[1] = gl_Vertex;
	gl_FrontColor = gl_Color;
}

