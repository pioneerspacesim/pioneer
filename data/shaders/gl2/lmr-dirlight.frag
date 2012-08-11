varying vec3 norm;
varying vec2 texCoord;
#ifdef TEXTURE
uniform sampler2D texture0;
#endif
#ifdef GLOWMAP
uniform sampler2D texture1;
#endif

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

#ifdef GLOWMAP
	vec4 emission = texture2D(texture1, texCoord);
#else
	vec4 emission = gl_FrontMaterial.emission;
#endif

	gl_FragColor =
		gl_LightModel.ambient * gl_FrontMaterial.ambient +
		amb * gl_FrontMaterial.ambient +
		diff * gl_FrontMaterial.diffuse +
		spec * gl_FrontMaterial.specular +
		emission;
	gl_FragColor.w = gl_FrontMaterial.diffuse.w;

#ifdef TEXTURE
	gl_FragColor *= texture2D(texture0, texCoord);
#endif

	SetFragDepth();
}
