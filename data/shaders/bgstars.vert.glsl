void main(void)
{
#ifdef ZHACK
	gl_Position = logarithmicTransform();
#else
	gl_Position = gl_ModelViewProjectionMatrix * gl_Vertex;
#endif
	gl_PointSize = 0.5 + 3.0*gl_Color.x*gl_Color.x*gl_Color.x;
	gl_FrontColor = gl_Color;
}

