void main(void)
{
	gl_Position = logarithmicTransform();
	gl_PointSize = 1.0 + 4.0*gl_Color.x*gl_Color.x*gl_Color.x;
	gl_FrontColor = gl_Color;
}

