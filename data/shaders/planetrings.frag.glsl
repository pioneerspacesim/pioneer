
void main(void)
{
	// Bits of ring in shadow!
	vec4 col = vec4(0.0);

	for (int i=0; i<NUM_LIGHTS; ++i) {
		float l = findSphereEyeRayEntryDistance(-vec3(gl_TexCoord[0]), vec3(gl_ModelViewMatrixInverse * gl_LightSource[i].position), 1.0);
		if (l <= 0.0) {
			col = col + gl_Color*gl_LightSource[i].diffuse;
		}
	}
	col.w = gl_Color.w;
	gl_FragColor = col;
#ifdef ZHACK
	SetFragDepth();
#endif
}
