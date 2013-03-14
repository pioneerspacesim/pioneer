varying vec3 varyingEyepos;
varying vec3 varyingNormal;
varying vec4 vertexColor;
varying vec4 unshadowed;

uniform vec3 geosphereCenter;
uniform float geosphereScaledRadius;

uniform int occultedLight;
uniform vec3 shadowCentre;
uniform float srad;
uniform float lrad;

#ifdef TERRAIN_WITH_LAVA
varying vec4 varyingEmission;
uniform Material material;
#endif

float intensityOfOccultedLight(vec3 lightDir, vec3 v, vec3 shadowCentre, float srad, float lrad) {
	vec3 projectedPoint = v - dot(lightDir,v)*lightDir;
	// By our assumptions, the proportion of light blocked at this point by
	// this sphere is the proportion of the disc of radius lrad around
	// projectedPoint covered by the disc of radius srad around shadowCentre.
	float dist = length(projectedPoint - shadowCentre);
	float maxOcclusion = srad > lrad ? 1.0 : srad*srad/lrad*lrad;

	return 1.0 - mix(0.0, maxOcclusion,
			clamp(
				( srad+lrad-dist ) / ( srad+lrad - abs(srad-lrad) ),
				0.0, 1.0));
}

void main(void)
{
	gl_Position = logarithmicTransform();
	vertexColor = gl_Color;
	varyingEyepos = vec3(gl_ModelViewMatrix * gl_Vertex);
	varyingNormal = gl_NormalMatrix * gl_Normal;

	// set unshadowed[i] to the effective intensity of light i:
	vec3 v = (varyingEyepos - geosphereCenter)/geosphereScaledRadius;
	float lenInvSq = 1.0/(dot(v,v));
	for (int i=0; i<NUM_LIGHTS; i++) {
		unshadowed[i] = 1.0;
		if (i == occultedLight)
			// Apply eclipse:
			unshadowed[i] *= intensityOfOccultedLight(lightDir, v, shadowCentre, srad, lrad);
	}

#ifdef TERRAIN_WITH_LAVA
	varyingEmission = material.emission;
	//Glow lava terrains
	if ( vertexColor.r > 0.4 && vertexColor.g < 0.2 && vertexColor.b < 0.4 ) {
		varyingEmission = 3.0*vertexColor;
		varyingEmission *= (vertexColor.r+vertexColor.g+vertexColor.b);

	}
#endif
}
