varying vec4 color;

void main(void)
{
	gl_FragColor = color;
#ifdef ZHACK
	SetFragDepth();
#endif
}
