varying vec3 norm;
uniform sampler2D tex;
uniform sampler2D texGlow;
uniform bool usetex;
uniform bool useglow;

void main(void)
{
	vec3 tnorm = normalize(norm);
	vec4 amb = vec4(0.0);
	vec4 diff = vec4(0.0);
	vec4 spec = vec4(0.0);
	for (int i=0; i<NUM_LIGHTS; ++i) {
		float nDotVP = max(0.0001, dot(tnorm, normalize(vec3(gl_LightSource[i].position))));
		float nDotHV = max(0.0001, dot(tnorm, vec3(gl_LightSource[i].halfVector)));
		float pf = max(0.0, pow(nDotHV, gl_FrontMaterial.shininess));
		amb += gl_LightSource[i].ambient;
		diff += gl_LightSource[i].diffuse * nDotVP;
		spec += gl_LightSource[i].specular * pf;
	}

	vec4 emission = gl_FrontMaterial.emission;
	if ( useglow )
		emission = texture2D(texGlow, gl_TexCoord[0].st);

	gl_FragColor = 
		gl_LightModel.ambient * gl_FrontMaterial.ambient +
		amb * gl_FrontMaterial.ambient +
		diff * gl_FrontMaterial.diffuse +
		spec * gl_FrontMaterial.specular +
		emission;
	gl_FragColor.w = gl_FrontMaterial.diffuse.w;

	if ( usetex )
		gl_FragColor *= texture2D(tex, gl_TexCoord[0].st);

#ifdef ZHACK
	SetFragDepth(gl_TexCoord[6].z);
#endif
}
