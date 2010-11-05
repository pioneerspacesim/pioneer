varying vec4 color;

void main(void)
{
#ifdef ZHACK
	gl_Position = logarithmicTransform();
#else
	gl_Position = gl_ModelViewProjectionMatrix * gl_Vertex;
#endif
	// not using gl_FrontColor because it gets clamped in vtx shaders
	color = gl_Color;
}

