void main(void)
{
#ifdef ZHACK
	gl_Position = logarithmicTransform();
#else
	gl_Position = gl_ModelViewProjectionMatrix * gl_Vertex;
#endif
	// not using gl_FrontColor because it gets clamped in vtx shaders
	gl_TexCoord[0] = gl_Color;
}

