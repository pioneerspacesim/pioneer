uniform vec4 atmosColor;
// to keep distances sane we do a nearer, smaller scam. this is how many times
// smaller the geosphere has been made
uniform float geosphereScale;
uniform float geosphereScaledRadius;
uniform float geosphereAtmosTopRad;
uniform vec3 geosphereCenter;
uniform float geosphereAtmosFogDensity;
uniform float geosphereAtmosInvScaleHeight;
uniform float currentDensity;

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
	float decaynormals,nDotVP,nnDotVP=0.0;
	float planetsurfaceDensity=geosphereAtmosFogDensity*10000.0;	
	float decay = max(1.0,sqrt(currentDensity)/2.5);  //density above darkens fast

	//Dense planet ?
	if (planetsurfaceDensity > 1.0) decaynormals = clamp(currentDensity/planetsurfaceDensity,0.0,1.0);
	else decaynormals = 1.0;

	#ifdef TERRAIN_WITH_WATER
	float specularReflection=0.0;
	#endif

#if (NUM_LIGHTS > 0)

	// when does the eye ray intersect atmosphere
	float atmosStart = findSphereEyeRayEntryDistance(geosphereCenter, eyepos, geosphereScaledRadius * geosphereAtmosTopRad);
	vec3 surfaceNorm = normalize(atmosStart*eyenorm - geosphereCenter);

	for (int i=0; i<NUM_LIGHTS; ++i) {	
		#ifdef ATMOSPHERE
		//use surfacenorm strength based on current density (normals decay), use dot vector from both sides to extend horizon.
		nDotVP  = max(0.0, dot(mix(surfaceNorm,tnorm,decaynormals), normalize(vec3(gl_LightSource[i].position))));
		nnDotVP = max(0.0, dot(mix(surfaceNorm,tnorm,decaynormals), normalize(-vec3(gl_LightSource[i].position)))); //need backlight to increase horizon
		#else
		nDotVP  = max(0.0, dot(tnorm, normalize(vec3(gl_LightSource[i].position))));
		nnDotVP = max(0.0, dot(tnorm, normalize(-vec3(gl_LightSource[i].position)))); //need backlight to increase horizon
		#endif
		diff += gl_LightSource[i].diffuse * 0.5*(nDotVP+0.8*clamp(1.0+nDotVP-nnDotVP*5.0,0.0,1.0)/float(NUM_LIGHTS));

		#ifdef TERRAIN_WITH_WATER
		//Specular reflection if water is present.
		vec3 L = normalize(gl_LightSource[i].position.xyz - eyepos); 
		vec3 E = normalize(-eyepos);
		vec3 R = normalize(-reflect(L,tnorm)); 
		//water only for specular (use colors)
	    	if (vertexColor.b > 0.05 && vertexColor.r < 0.05) {
			specularReflection += pow(max(dot(R,E),0.0),16.0)*0.4/float(NUM_LIGHTS);
		}
		#endif
	}

	#ifdef ATMOSPHERE
	float ldprod=0.0;
	float fogFactor=0.0;
	{
		float atmosDist = geosphereScale * (length(eyepos) - atmosStart);
		
		// a&b scaled so length of 1.0 means planet surface.
		vec3 a = (atmosStart * eyenorm - geosphereCenter) / geosphereScaledRadius;
		vec3 b = (eyepos - geosphereCenter) / geosphereScaledRadius;
		ldprod = AtmosLengthDensityProduct(a, b, atmosColor.w*geosphereAtmosFogDensity, atmosDist, geosphereAtmosInvScaleHeight);
		fogFactor = clamp( 1.5 / exp(ldprod),0.0,1.0); 
	}

	//calculate sunset tone red when passing through more atmosphere, clamp everything.
	float atmpower = (diff.r+diff.g+diff.b)/3.0;
	vec4 sunset = vec4(1.0,clamp(pow(atmpower,0.8),0.0,1.0),clamp(pow(atmpower,1.2),0.0,1.0),1.0);

	gl_FragColor =
		material.emission +
		#ifdef TERRAIN_WITH_LAVA
		varyingEmission +
		#endif
		fogFactor *
		((scene.ambient * vertexColor) +
		(diff * vertexColor)) +
		(1.0-fogFactor)*(diff*atmosColor) +
		  #ifdef TERRAIN_WITH_WATER
		  diff*specularReflection*sunset +
		  #endif
		  ((0.02-clamp(fogFactor,0.0,0.01))*diff*ldprod*sunset/max(1.0,pow(planetsurfaceDensity,4.0)))/decay +	      //increase fog scatter				
		  ((pow((1.0-pow(fogFactor,0.75)),256.0)*0.4*diff*atmosColor)*sunset/max(1.0,pow(planetsurfaceDensity,4.0)))/decay;  //distant fog.

	//decay light in thick atmosphere and tone red
	gl_FragColor = vec4((clamp(gl_FragColor.r,0.0,1.0)/decay)+0.25*(1.0-1.0/decay),
			    (clamp(gl_FragColor.g,0.0,1.0)/decay)+0.10*(1.0-1.0/decay),
                            (clamp(gl_FragColor.b,0.0,1.0)/decay)-0.99*(1.0-1.0/decay)		,
                            gl_FragColor.a);

	#else // atmosphere-less planetoids and dim stars
	gl_FragColor =
		material.emission +
		#ifdef TERRAIN_WITH_LAVA
		varyingEmission +
		#endif
		(scene.ambient * vertexColor) +
		(diff * vertexColor * 2.0);
	#endif //ATMOSPHERE

#else // NUM_LIGHTS > 0 -- unlit rendering - stars
	//emission is used to boost colour of stars, which is a bit odd
	gl_FragColor = material.emission + vertexColor;
#endif
	SetFragDepth();
}
