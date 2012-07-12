uniform vec4 atmosColor;
// to keep distances sane we do a nearer, smaller scam. this is how many times
// smaller the geosphere has been made
uniform float geosphereScale;
uniform float geosphereScaledRadius;
uniform float geosphereAtmosTopRad;
uniform vec3 geosphereCenter;
uniform float geosphereAtmosFogDensity;
uniform float geosphereAtmosInvScaleHeight;

varying vec3 varyingEyepos;
varying vec3 varyingNormal;

void main(void)
{
	vec3 eyepos = varyingEyepos;
	vec3 eyenorm = normalize(eyepos);
	vec3 tnorm = normalize(varyingNormal);
	vec4 diff = vec4(0.0);

	for (int i=0; i<NUM_LIGHTS; ++i) {
		float nDotVP = max(0.0, dot(tnorm, normalize(vec3(gl_LightSource[i].position))));
		diff += gl_LightSource[i].diffuse * nDotVP;
	}

	// when does the eye ray intersect atmosphere
	float atmosStart = findSphereEyeRayEntryDistance(geosphereCenter, eyepos, geosphereScaledRadius * geosphereAtmosTopRad);

	float fogFactor;
	{
		float atmosDist = geosphereScale * (length(eyepos) - atmosStart);
		float ldprod;
		// a&b scaled so length of 1.0 means planet surface.
		vec3 a = (atmosStart * eyenorm - geosphereCenter) / geosphereScaledRadius;
		vec3 b = (eyepos - geosphereCenter) / geosphereScaledRadius;
		ldprod = AtmosLengthDensityProduct(a, b, atmosColor.w*geosphereAtmosFogDensity, atmosDist, geosphereAtmosInvScaleHeight);
		fogFactor = 1.0 / exp(ldprod);
	}

	vec4 atmosDiffuse = vec4(0.0,0.0,0.0,1.0);
	{
		vec3 surfaceNorm = normalize(atmosStart*eyenorm - geosphereCenter);
		for (int i=0; i<NUM_LIGHTS; ++i) {
			atmosDiffuse += gl_LightSource[i].diffuse * max(0.0, dot(surfaceNorm, normalize(vec3(gl_LightSource[i].position))));
		}
	}
	atmosDiffuse.a = 1.0;

	gl_FragColor = (fogFactor) * ((diff)*gl_Color + gl_LightModel.ambient*gl_Color) +
		(1.0-fogFactor)*(atmosDiffuse*atmosColor) + gl_FrontMaterial.emission;

#ifdef ZHACK
	SetFragDepth(gl_TexCoord[6].z);
#endif
}
