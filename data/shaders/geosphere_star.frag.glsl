void main(void)
{
#ifdef DIM
	vec3 tnorm = normalize(vec3(gl_TexCoord[1]));
	vec4 diff = vec4(0.0);
	for (int i=0; i<NUM_LIGHTS; ++i) {
		float nDotVP = max(0.0, dot(tnorm, normalize(vec3(gl_LightSource[i].position))));
		diff += gl_LightSource[i].diffuse * nDotVP;
	}
#else
	vec4 diff = vec4(1.0);
#endif

	gl_FragColor = diff*gl_Color + gl_LightModel.ambient*gl_Color;
	gl_FragColor *= gl_FrontMaterial.emission * gl_FrontMaterial.emission * 10.0;
	//gl_FragColor = vec4(clamp(gl_FragColor.x, 0.0, 10000.0), clamp(gl_FragColor.y, 0.0, 10000.0), //clamp(gl_FragColor.z, 0.0, 10000.0), clamp(gl_FragColor.w, 0.0, 100.0));

#ifdef ZHACK
	SetFragDepth(gl_TexCoord[6].z);
#endif
}
