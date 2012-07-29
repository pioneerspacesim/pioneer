varying vec4 color;

void main(void)
{
	gl_Position = logarithmicTransform();

	// not using gl_FrontColor because it gets clamped in vtx shaders
	color = gl_Color;
}

