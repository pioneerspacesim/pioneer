varying vec3 norm;
uniform sampler2D tex;
uniform bool usetex;

void main(void)
{
	vec3 eye = vec3(0.0, 0.0, 0.0);//gl_TexCoord[2]);
	vec3 ecPosition3 = vec3(gl_TexCoord[1]);
	vec3 tnorm = normalize(norm);
	vec4 amb = vec4(0.0);
	vec4 diff = vec4(0.0);
	vec4 spec = vec4(0.0);
//	for (int i=4; i<8; ++i) {
		PointLight(4, eye, ecPosition3, tnorm, amb, diff, spec);
//	}
	gl_FragColor = gl_FrontLightModelProduct.sceneColor +
		amb * gl_FrontMaterial.ambient +
		diff * gl_FrontMaterial.diffuse +
		spec * gl_FrontMaterial.specular;

	if ( usetex )
		gl_FragColor *= texture2D(tex, gl_TexCoord[0].st);

	SetFragDepth(gl_TexCoord[6].z);
}
