void main(void)
{
	gl_TexCoord[0] = gl_Vertex;
	gl_Position = logarithmicTransform();
	gl_FrontColor = gl_Color;
}

