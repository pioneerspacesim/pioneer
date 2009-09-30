void main(void)
{
	gl_Position = logarithmicTransform();
//	vec4 vertexPosClip = gl_ModelViewProjectionMatrix * gl_Vertex;
//	gl_Position = vertexPosClip;
//	gl_TexCoord[6] = vertexPosClip;

	vec3 eyepos = vec3(gl_ModelViewMatrix * gl_Vertex);
	vec3 tnorm = normalize(gl_NormalMatrix * gl_Normal);
	vec4 amb = vec4(0.0);
	vec4 diff = vec4(0.0);
	vec4 spec = vec4(0.0);
	for (int i=0; i<NUM_LIGHTS; ++i) {
		DirectionalLight(i, tnorm, amb, diff, spec);
	}
	gl_FrontColor = gl_FrontLightModelProduct.sceneColor +
		amb * gl_FrontMaterial.ambient +
		diff * gl_FrontMaterial.diffuse +
		spec * gl_FrontMaterial.specular;
}
