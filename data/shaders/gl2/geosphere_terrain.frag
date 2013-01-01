uniform vec4 atmosColor;
// to keep distances sane we do a nearer, smaller scam. this is how many times
// smaller the geosphere has been made
uniform float geosphereScale;
uniform float geosphereScaledRadius;
uniform float geosphereAtmosTopRad;
uniform vec3 geosphereCenter;
uniform float geosphereAtmosFogDensity;
uniform float geosphereAtmosInvScaleHeight;

uniform Material material;
uniform Scene scene;

varying vec3 varyingEyepos;
varying vec3 varyingNormal;
varying vec4 vertexColor;

#ifdef TERRAIN_WITH_LAVA
varying vec4 varyingEmission;
#endif

void main(void)
{
	vec3 eyepos = varyingEyepos;
	vec3 eyenorm = normalize(eyepos);
	vec3 tnorm = normalize(varyingNormal);
	vec4 diff = vec4(0.0);
	float nDotVP;
	float nnDotVP;
#ifdef TERRAIN_WITH_WATER
	float specularReflection;
#endif

#if (NUM_LIGHTS > 0)
	for (int i=0; i<NUM_LIGHTS; ++i) {
		nDotVP  = max(0.0, dot(tnorm, normalize(vec3(gl_LightSource[i].position))));
		nnDotVP = max(0.0, dot(tnorm, normalize(-vec3(gl_LightSource[i].position)))); //need backlight to increase horizon
		diff += 0.5*(nDotVP+0.5*clamp(1.0-nnDotVP*4.0,0.0,1.0)/float(NUM_LIGHTS));

#ifdef TERRAIN_WITH_WATER
		//Specular reflection
		vec3 L = normalize(gl_LightSource[i].position.xyz - eyepos); 
		vec3 E = normalize(-eyepos);
		vec3 R = normalize(-reflect(L,tnorm)); 
		//water only for specular
	    	if (vertexColor.b > 0.05 && vertexColor.r < 0.05) {
			specularReflection += pow(max(dot(R,E),0.0),16.0)*0.4/float(NUM_LIGHTS);
		}
#endif
	}

#ifdef ATMOSPHERE
	// when does the eye ray intersect atmosphere
	float atmosStart = findSphereEyeRayEntryDistance(geosphereCenter, eyepos, geosphereScaledRadius * geosphereAtmosTopRad);
	float ldprod;
	float fogFactor;
	{
		float atmosDist = geosphereScale * (length(eyepos) - atmosStart);
		
		// a&b scaled so length of 1.0 means planet surface.
		vec3 a = (atmosStart * eyenorm - geosphereCenter) / geosphereScaledRadius;
		vec3 b = (eyepos - geosphereCenter) / geosphereScaledRadius;
		ldprod = AtmosLengthDensityProduct(a, b, atmosColor.w*geosphereAtmosFogDensity, atmosDist, geosphereAtmosInvScaleHeight);
		fogFactor = clamp( 1.5 / exp(ldprod),0.0,1.0); 
	}

	vec4 atmosDiffuse = vec4(0.0);
	{
		vec3 surfaceNorm = normalize(atmosStart*eyenorm - geosphereCenter);
		for (int i=0; i<NUM_LIGHTS; ++i) {
			atmosDiffuse += gl_LightSource[i].diffuse * 0.5*(nDotVP+0.5*clamp(1.0-nnDotVP*4.0,0.0,1.0)/float(NUM_LIGHTS));
		}
	}

	//calculate sunset tone red when passing through more atmosphere, clamp everything.
	float atmpower = (atmosDiffuse.r+atmosDiffuse.g+atmosDiffuse.b)/3.0;
	vec4 sunset = vec4(0.8,clamp(pow(atmpower,0.8),0.0,1.0),clamp(pow(atmpower,1.2),0.0,1.0),1.0);

	atmosDiffuse.a = 1.0;

	gl_FragColor =
		material.emission +
#ifdef TERRAIN_WITH_LAVA
		varyingEmission +
#endif
		fogFactor *
		((scene.ambient * vertexColor) +
		(diff * vertexColor)) +
		(1.0-fogFactor)*(atmosDiffuse*atmosColor) +
#ifdef TERRAIN_WITH_WATER
		  diff*specularReflection*sunset +
#endif
		  (0.02-clamp(fogFactor,0.0,0.01))*atmosDiffuse*ldprod*sunset +	      //increase fog scatter				
		  (pow((1.0-pow(fogFactor,0.75)),256.0)*0.4*diff*atmosColor)*sunset;  //distant fog.
#else // atmosphere-less planetoids and dim stars
	gl_FragColor =
		material.emission +
#ifdef TERRAIN_WITH_LAVA
		varyingEmission +
#endif
		(scene.ambient * vertexColor) +
		(diff * vertexColor);
#endif //ATMOSPHERE

#else // NUM_LIGHTS > 0 -- unlit rendering - stars
	//emission is used to boost colour of stars, which is a bit odd
	gl_FragColor = material.emission + vertexColor;
#endif
	SetFragDepth();
}
