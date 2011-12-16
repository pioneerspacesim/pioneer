uniform vec3 geosphereCenter;
uniform float geosphereRadius;

uniform int occultedLight;
uniform vec3 occultCentre;
uniform float srad;
uniform float lrad;
uniform float maxOcclusion;
uniform vec4 lightDiscRadii;

varying vec4 varyingEyepos;

void main(void)
{
#ifdef ZHACK
	gl_Position = logarithmicTransform();
#else
	gl_Position = gl_ModelViewProjectionMatrix * gl_Vertex;
#endif
	varyingEyepos = gl_ModelViewMatrix * gl_Vertex;

	// as in geosphere.vert.glsl
	vec3 v = (varyingEyepos - geosphereCenter)/geosphereRadius;
	float lenInvSq = 1.0/(length(v)*length(v));
	for (int i=0; i<NUM_LIGHTS; i++) {
		vec3 lightDir = normalize(gl_LightSource[i].position - geosphereCenter);
		float perp = dot(lightDir,v);
		float d = perp*lenInvSq + sqrt((1-lenInvSq)*(1-(perp*perp*lenInvSq)));
		if (lightDiscRadii[i] < 0.0)
			gl_TexCoord[2][i] = 1.0;
		else
			gl_TexCoord[2][i] = clamp(d / (2*lightDiscRadii[i]) + 0.5, 0.0, 1.0);
		if (i == occultedLight)
			gl_TexCoord[2][i]*=intensityOfOccultedLight(lightDir, v, occultCentre, srad, lrad, maxOcclusion);
	}
}
