uniform float invLogZfarPlus1;

void main(void)
{
//	gl_FragDepth = gl_DepthRange.near + log2(gl_TexCoord[6].z + 1.0) * invLogZfarPlus1;
	gl_FragColor = gl_Color;
}
