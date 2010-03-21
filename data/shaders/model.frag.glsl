varying vec3 norm;
uniform sampler2D tex;
uniform bool usetex;

void main(void)
{
	vec3 tnorm = normalize(norm);
	vec4 amb = vec4(0.0);
	vec4 diff = vec4(0.0);
	vec4 spec = vec4(0.0);
#if defined(VENDOR_R300_GALLIUM)
	{ /* for the love of a buggy alpha-stage driver :) */
		float nDotVP; 
		float nDotHV;         
		nDotVP = max(0.0, dot(tnorm, normalize(vec3(gl_LightSource[0].position))));
		nDotHV = max(0.0, dot(tnorm, vec3(gl_LightSource[0].halfVector)));
		if (nDotVP != 0.0) {
			float pf = pow(nDotHV, gl_FrontMaterial.shininess);
			spec += gl_LightSource[0].specular * pf;
		}
		amb += gl_LightSource[0].ambient;
		diff += gl_LightSource[0].diffuse * nDotVP;
	}
#else
	for (int i=0; i<NUM_LIGHTS; ++i) {
		DirectionalLight(i, tnorm, amb, diff, spec);
	}
#endif
	gl_FragColor = gl_FrontLightModelProduct.sceneColor +
		amb * gl_FrontMaterial.ambient +
		diff * gl_FrontMaterial.diffuse +
		spec * gl_FrontMaterial.specular;

	if ( usetex )
		gl_FragColor *= texture2D(tex, gl_TexCoord[0].st);

	SetFragDepth(gl_TexCoord[6].z);
}
