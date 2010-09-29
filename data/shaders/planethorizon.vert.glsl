void main(void)
{
#ifdef ZHACK
	gl_Position = logarithmicTransform();
#else
	gl_Position = ftransform();
#endif
	vec3 tnorm = normalize(gl_NormalMatrix * gl_Normal);
	float diffuse = 0.0;
	for (int i=0; i<NUM_LIGHTS; ++i) {
		diffuse += max(0.0, dot(tnorm, normalize(vec3(gl_LightSource[i].position))));
	}
	gl_FrontColor = diffuse * gl_Color;
}

