void main(void)
{
	gl_Position = logarithmicTransform();

	gl_PointSize = 1.0 + pow(gl_Color.r,3.0);
	gl_FrontColor = gl_Color * gl_FrontMaterial.emission;
}

