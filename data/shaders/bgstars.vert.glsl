void main(void)
{
#ifdef ZHACK
	gl_Position = logarithmicTransform();
#else
	gl_Position = gl_ModelViewProjectionMatrix * gl_Vertex;
#endif
	gl_PointSize = 1.0 + pow(gl_Color.r,3.0);
	gl_FrontColor = gl_Color;
}

