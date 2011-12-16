uniform vec3 geosphereCenter;
uniform float geosphereRadius;

uniform int occultedLight;
uniform vec3 occultCentre;
uniform float srad;
uniform float lrad;
uniform float maxOcclusion;
uniform vec4 lightDiscRadii;

void main(void)
{
#ifdef ZHACK
	gl_Position = logarithmicTransform();
#else
	gl_Position = gl_ModelViewProjectionMatrix * gl_Vertex;
#endif


	gl_FrontColor = gl_Color;
	gl_TexCoord[0] = gl_ModelViewMatrix * gl_Vertex;
	vec3 tnorm = gl_NormalMatrix * gl_Normal;
	gl_TexCoord[1] = vec4(tnorm.x, tnorm.y, tnorm.z, 0.0);

	// set gl_TexCoord[2][i] to the effective intensity of light i:
	vec3 v = (gl_TexCoord[0] - geosphereCenter)/geosphereRadius;
	float lenInvSq = 1.0/(length(v)*length(v));
	for (int i=0; i<NUM_LIGHTS; i++) {
		vec3 lightDir = normalize(gl_LightSource[i].position - geosphereCenter);

		// Handle self-shadowing, i.e. "night".
		// d = dot(lightDir,t) where t is the unique point on the unit sphere whose tangent plane
		// contains v, is in the plane of lightDir and d, and is towards the light.
		float perp = dot(lightDir,v);
		float d = perp*lenInvSq + sqrt((1-lenInvSq)*(1-(perp*perp*lenInvSq)));
		if (lightDiscRadii[i] < 0.0)
			gl_TexCoord[2][i] = 1.0;
		else
			// Just linearly interpolate (the correct calculation involves
			// asin, which isn't so cheap)
			gl_TexCoord[2][i] = clamp(d / (2*lightDiscRadii[i]) + 0.5, 0.0, 1.0);

		if (i == occultedLight)
			// Apply eclipse:
			// TODO: should be branching in a way the shader compiler can understand (uniform bool? diff shader?)
			gl_TexCoord[2][i]*=intensityOfOccultedLight(lightDir, v, occultCentre, srad, lrad, maxOcclusion);
	}
}
