// Copyright © 2008-2014 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

uniform vec4 atmosColor;
// to keep distances sane we do a nearer, smaller scam. this is how many times
// smaller the geosphere has been made
uniform float geosphereScale;
uniform float geosphereScaledRadius;
uniform float geosphereAtmosTopRad;
uniform vec3 geosphereCenter;
uniform float geosphereAtmosFogDensity;
uniform float geosphereAtmosInvScaleHeight;

#ifdef ECLIPSE
uniform int shadows;
uniform ivec3 occultedLight;
uniform vec3 shadowCentreX;
uniform vec3 shadowCentreY;
uniform vec3 shadowCentreZ;
uniform vec3 srad;
uniform vec3 lrad;
uniform vec3 sdivlrad;
#endif // ECLIPSE

uniform Material material;
uniform Scene scene;

varying vec3 varyingEyepos;
varying vec3 varyingNormal;
varying vec4 vertexColor;

#ifdef TERRAIN_WITH_LAVA
varying vec4 varyingEmission;
#endif

#ifdef ECLIPSE
#define PI 3.141592653589793

float discCovered(const in float dist, const in float rad) {
	// proportion of unit disc covered by a second disc of radius rad placed
	// dist from centre of first disc.
	//
	// XXX: same function is in Camera.cpp
	//
	// WLOG, the second disc is displaced horizontally to the right.
	// xl = rightwards distance to intersection of the two circles.
	// xs = normalised leftwards distance from centre of second disc to intersection.
	// d = vertical distance to an intersection point
	//
	// The clamps on xl,xs handle the cases where one disc contains the other.

	float radsq = rad*rad;

	float xl = clamp((dist*dist + 1.0 - radsq) / (2.0*max(0.001,dist)), -1.0, 1.0);
	float xs = clamp((dist - xl)/max(0.001,rad), -1.0, 1.0);
	float d = sqrt(max(0.0, 1.0 - xl*xl));

	float th = clamp(acos(xl), 0.0, PI);
	float th2 = clamp(acos(xs), 0.0, PI);

	// covered area can be calculated as the sum of segments from the two
	// discs plus/minus some triangles, and it works out as follows:
	return clamp((th + radsq*th2 - dist*d)/PI, 0.0, 1.0);
}
#endif // ECLIPSE

void main(void)
{
	vec3 eyepos = varyingEyepos;
	vec3 eyenorm = normalize(eyepos);
	vec3 tnorm = normalize(varyingNormal);
	vec4 diff = vec4(0.0);
	float nDotVP=0.0;
	float nnDotVP=0.0;
#ifdef TERRAIN_WITH_WATER
	float specularReflection=0.0;
#endif

#if (NUM_LIGHTS > 0)
	vec3 v = (eyepos - geosphereCenter)/geosphereScaledRadius;
	float lenInvSq = 1.0/(dot(v,v));
	for (int i=0; i<NUM_LIGHTS; ++i) {
		vec3 lightDir = normalize(vec3(gl_LightSource[i].position));
		float unshadowed = 1.0;
#ifdef ECLIPSE
		for (int j=0; j<shadows; j++) {
			if (i != occultedLight[j])
				continue;
				
			vec3 centre = vec3( shadowCentreX[j], shadowCentreY[j], shadowCentreZ[j] );
			
			// Apply eclipse:
			vec3 projectedPoint = v - dot(lightDir,v)*lightDir;
			// By our assumptions, the proportion of light blocked at this point by
			// this sphere is the proportion of the disc of radius lrad around
			// projectedPoint covered by the disc of radius srad around shadowCentre.
			float dist = length(projectedPoint - centre);
			unshadowed *= 1.0 - discCovered(dist/lrad[j], sdivlrad[j]);
		}
#endif // ECLIPSE
		unshadowed = clamp(unshadowed, 0.0, 1.0);
		nDotVP  = max(0.0, dot(tnorm, normalize(vec3(gl_LightSource[i].position))));
		nnDotVP = max(0.0, dot(tnorm, normalize(-vec3(gl_LightSource[i].position)))); //need backlight to increase horizon
		diff += gl_LightSource[i].diffuse * unshadowed * 0.5*(nDotVP+0.5*clamp(1.0-nnDotVP*4.0,0.0,1.0) * INV_NUM_LIGHTS);

#ifdef TERRAIN_WITH_WATER
		//Specular reflection
		vec3 L = normalize(gl_LightSource[i].position.xyz - eyepos); 
		vec3 E = normalize(-eyepos);
		vec3 R = normalize(-reflect(L,tnorm)); 
		//water only for specular
	    if (vertexColor.b > 0.05 && vertexColor.r < 0.05) {
			specularReflection += pow(max(dot(R,E),0.0),16.0)*0.4 * INV_NUM_LIGHTS;
		}
#endif
	}

#ifdef ATMOSPHERE
	// when does the eye ray intersect atmosphere
	float atmosStart = findSphereEyeRayEntryDistance(geosphereCenter, eyepos, geosphereScaledRadius * geosphereAtmosTopRad);
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
	vec4 sunset = vec4(0.8,clamp(pow(atmpower,0.8),0.0,1.0),clamp(pow(atmpower,1.2),0.0,1.0),1.0);

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
		  (0.02-clamp(fogFactor,0.0,0.01))*diff*ldprod*sunset +	      //increase fog scatter				
		  (pow((1.0-pow(fogFactor,0.75)),256.0)*0.4*diff*atmosColor)*sunset;  //distant fog.
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
