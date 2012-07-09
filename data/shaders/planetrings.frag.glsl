
void main(void)
{
	// Bits of ring in shadow!
	vec4 col = vec4(0.0);

	for (int i=0; i<NUM_LIGHTS; ++i) {
		float l = findSphereEyeRayEntryDistance(-vec3(gl_TexCoord[0]), vec3(gl_ModelViewMatrixInverse * gl_LightSource[i].position), 1.0);
		
		//if (l <= 0.0) {
		//	col = col + gl_Color*gl_LightSource[i].diffuse;
		//}

		float intensity = (clamp(-l,0.0,0.00000001)*(0.7/0.00000001));
		//float intensity = (l <= 0.0)?1.0:0.3;
		col = col+gl_Color*vec4(gl_LightSource[i].diffuse.rgb*intensity,gl_LightSource[i].diffuse.a);

	}
	col.w = gl_Color.w;
	gl_FragColor = col;
#ifdef ZHACK
	SetFragDepth(gl_TexCoord[6].z);
#endif
}
