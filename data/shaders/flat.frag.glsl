uniform vec4 color;

void main(void)
{
	gl_FragColor = color;
	SetFragDepth(gl_TexCoord[6].z);
}
