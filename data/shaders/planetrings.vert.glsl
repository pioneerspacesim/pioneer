void main(void)
{
#ifdef ZHACK
	gl_Position = logarithmicTransform();
#else
	gl_Position = ftransform();
#endif
	gl_TexCoord[0] = gl_Vertex;
	gl_FrontColor = gl_Color;
}

