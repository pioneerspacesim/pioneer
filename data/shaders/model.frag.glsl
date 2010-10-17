varying vec3 norm;
uniform sampler2D tex;
uniform bool usetex;

void main(void)
{
	vec3 tnorm = normalize(norm);
	vec4 amb = vec4(0.0);
	vec4 diff = vec4(0.0);
	vec4 spec = vec4(0.0);
	for (int i=0; i<NUM_LIGHTS; ++i) {
		float nDotVP; 
		float nDotHV;         
		float pf;             
		nDotVP = max(0.0, dot(tnorm, normalize(vec3(gl_LightSource[i].position))));
		nDotHV = max(0.0, dot(tnorm, vec3(gl_LightSource[i].halfVector)));
		if (nDotVP == 0.0) pf = 0.0;
		else pf = pow(nDotHV, gl_FrontMaterial.shininess);
		amb += gl_LightSource[i].ambient;
		diff += gl_LightSource[i].diffuse * nDotVP;
		spec += gl_LightSource[i].specular * pf;
	}
	gl_FragColor = 
		gl_LightModel.ambient * gl_FrontMaterial.ambient +
		amb * gl_FrontMaterial.ambient +
		diff * gl_FrontMaterial.diffuse +
		spec * gl_FrontMaterial.specular +
		gl_FrontMaterial.emission;
	gl_FragColor.w = gl_FrontMaterial.diffuse.w;

	if ( usetex )
		gl_FragColor *= texture2D(tex, gl_TexCoord[0].st);

#ifdef ZHACK
	SetFragDepth(gl_TexCoord[6].z);
#endif
}
