void main(void)
{
#ifdef ZHACK
	gl_Position = logarithmicTransform();
#else
	gl_Position = ftransform();
#endif
	gl_FrontColor = gl_Color;
}

