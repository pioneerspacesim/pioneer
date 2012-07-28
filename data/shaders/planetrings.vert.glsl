void main(void)
{
	gl_Position = logarithmicTransform();

	gl_TexCoord[0] = gl_MultiTexCoord0;
	gl_TexCoord[1] = gl_Vertex;

	gl_FrontColor = gl_Color;
}

