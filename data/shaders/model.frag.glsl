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
		DirectionalLight(i, tnorm, amb, diff, spec);
	}
	gl_FragColor = gl_FrontLightModelProduct.sceneColor +
		amb * gl_FrontMaterial.ambient +
		diff * gl_FrontMaterial.diffuse +
		spec * gl_FrontMaterial.specular;

	if ( usetex )
		gl_FragColor *= texture2D(tex, gl_TexCoord[0].st);

	SetFragDepth(gl_TexCoord[6].z);
}
