void main(void)
{
	gl_Position = logarithmicTransform();

	gl_FrontColor = gl_Color;
	gl_TexCoord[0] = gl_MultiTexCoord0;
}

