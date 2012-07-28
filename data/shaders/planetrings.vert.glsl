void main(void)
{
	gl_Position = logarithmicTransform();

	gl_TexCoord[0] = gl_Vertex;
	gl_FrontColor = gl_Color;
}

