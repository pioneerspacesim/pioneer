uniform vec4 atmosColor;
// to keep distances sane we do a nearer, smaller scam. this is how many times
// smaller the geosphere has been made
uniform float geosphereScale;
uniform float geosphereAtmosTopRad;
uniform vec3 geosphereCenter;
uniform float geosphereAtmosFogDensity;

void main(void)
{
	gl_Position = logarithmicTransform();
	vec3 eyepos = vec3(gl_ModelViewMatrix * gl_Vertex);
	vec3 tnorm = normalize(gl_NormalMatrix * gl_Normal);
	vec4 amb = vec4(0.0);
	vec4 diff = vec4(0.0);
	for (int i=0; i<NUM_LIGHTS; ++i) {
		DirectionalLight(i, tnorm, amb, diff);
	}

	// when does the eye ray intersect atmosphere
	float atmosStart = findSphereEyeRayEntryDistance(geosphereCenter, eyepos, geosphereAtmosTopRad);
	
	float fogFactor;
	{
		float atmosDist = geosphereScale * (length(eyepos) - atmosStart);
		fogFactor = 1.0 / exp(geosphereAtmosFogDensity * atmosDist);
	}

	float atmosDiffuse = 0.0;
	{
		vec3 surfaceNorm = normalize(eyepos - geosphereCenter);
		for (int i=0; i<NUM_LIGHTS; ++i) {
			atmosDiffuse += max(0.0, dot(surfaceNorm, normalize(vec3(gl_LightSource[i].position))));
		}
	}
	gl_FrontColor = gl_FrontLightModelProduct.sceneColor + (fogFactor)*(amb+diff)*gl_Color + (1.0-fogFactor)*atmosDiffuse*atmosColor;
}
