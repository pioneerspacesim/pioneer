void main(void)
{
	gl_FragColor = gl_Color + gl_LightModel.ambient*gl_Color;
	gl_FragColor *= gl_FrontMaterial.emission * gl_FrontMaterial.emission * 10.0;

#ifdef ZHACK
	SetFragDepth(gl_TexCoord[6].z);
#endif
}
