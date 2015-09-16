// Copyright © 2008-2015 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

uniform vec4 atmosColor;
// to keep distances sane we do a nearer, smaller scam. this is how many times
// smaller the geosphere has been made
uniform float geosphereRadius;
uniform float geosphereAtmosTopRad;
uniform vec3 geosphereCenter;
uniform float geosphereAtmosFogDensity;
uniform float geosphereAtmosInvScaleHeight;

uniform sampler2D texture0;	// hi detail
uniform sampler2D texture1;	// lo detail
uniform sampler2D texture2;	// lookup
uniform sampler2DArray texture3; // atlas
in vec2 texCoord0;
in vec2 slopeHeight;

in float dist;
uniform float detailScaleHi;
uniform float detailScaleLo;

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

in vec3 varyingEyepos;
in vec3 varyingNormal;

#ifdef TERRAIN_WITH_LAVA
in vec4 varyingEmission;
#endif

out vec4 frag_color;

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
	vec4 hidetail = texture(texture0, texCoord0 * detailScaleHi);
	vec4 lodetail = texture(texture1, texCoord0 * detailScaleLo);
	vec3 eyepos = varyingEyepos;
	vec3 eyenorm = normalize(eyepos);
	vec3 tnorm = normalize(varyingNormal);
	vec4 diff = vec4(0.0);

	// calculte the detail texture contribution from hi and lo textures
	float hiloMix = exp(-0.004 * dist);
	float detailMix = exp(-0.001 * dist);
	vec4 detailVal = mix(lodetail, hidetail, hiloMix);
	vec4 detailMul = mix(vec4(1.0), detailVal, detailMix);

	float nDotVP=0.0;
	float nnDotVP=0.0;
#ifdef TERRAIN_WITH_WATER
	float specularReflection=0.0;
#endif

#if (NUM_LIGHTS > 0)
	vec3 v = (eyepos - geosphereCenter)/geosphereRadius;
	float lenInvSq = 1.0/(dot(v,v));
	for (int i=0; i<NUM_LIGHTS; ++i) {
		vec3 lightDir = normalize(vec3(uLight[i].position));
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
		nDotVP  = max(0.0, dot(tnorm, normalize(vec3(uLight[i].position))));
		nnDotVP = max(0.0, dot(tnorm, normalize(-vec3(uLight[i].position)))); //need backlight to increase horizon
		diff += uLight[i].diffuse * unshadowed * 0.5*(nDotVP+0.5*clamp(1.0-nnDotVP*4.0,0.0,1.0) * INV_NUM_LIGHTS);

#ifdef TERRAIN_WITH_WATER
		//Specular reflection
		/*vec3 L = normalize(uLight[i].position.xyz - eyepos); 
		vec3 E = normalize(-eyepos);
		vec3 R = normalize(-reflect(L,tnorm)); 
		//water only for specular
	    if (vertexColor.b > 0.05 && vertexColor.r < 0.05) {
			specularReflection += pow(max(dot(R,E),0.0),16.0)*0.4 * INV_NUM_LIGHTS;
		}*/
#endif
	}
	
#ifdef TEXTURE0
	vec4 color = vec4( 0.125, 0.25, 1.0, 1.0);
	// LookupTexture & slope/height texture coords
	float terrainType = texture(texture0, slopeHeight).x;
	// mul by 16 because values are 0..1 but we need them 0..15 for array indexing
	int id0 = int(terrainType * 16.0); 
	color = texture(texture3, vec3(texCoord0 * detailScaleLo, float(id0))).rgba;
#else
	vec4 color = vec4(0.313, 0.313, 0.313, 1.0);
#endif

	// Use the detail value to multiply the final colour before lighting
	vec4 final = color * detailMul;
	
#ifdef ATMOSPHERE
	// when does the eye ray intersect atmosphere
	float atmosStart = findSphereEyeRayEntryDistance(geosphereCenter, eyepos, geosphereRadius * geosphereAtmosTopRad);
	float ldprod=0.0;
	float fogFactor=0.0;
	{
		float atmosDist = (length(eyepos) - atmosStart);
		
		// a&b scaled so length of 1.0 means planet surface.
		vec3 a = (atmosStart * eyenorm - geosphereCenter) / geosphereRadius;
		vec3 b = (eyepos - geosphereCenter) / geosphereRadius;
		ldprod = AtmosLengthDensityProduct(a, b, atmosColor.w*geosphereAtmosFogDensity, atmosDist, geosphereAtmosInvScaleHeight);
		fogFactor = clamp( 1.5 / exp(ldprod),0.0,1.0); 
	}

	//calculate sunset tone red when passing through more atmosphere, clamp everything.
	float atmpower = (diff.r+diff.g+diff.b)/3.0;
	vec4 sunset = vec4(0.8,clamp(pow(atmpower,0.8),0.0,1.0),clamp(pow(atmpower,1.2),0.0,1.0),1.0);

	frag_color =
		material.emission +
#ifdef TERRAIN_WITH_LAVA
		varyingEmission +
#endif
		fogFactor *
		((scene.ambient * color) +
		(diff * final)) +
		(1.0-fogFactor)*(diff*atmosColor) +
#ifdef TERRAIN_WITH_WATER
		  diff*specularReflection*sunset +
#endif
		  (0.02-clamp(fogFactor,0.0,0.01))*diff*ldprod*sunset +	      //increase fog scatter				
		  (pow((1.0-pow(fogFactor,0.75)),256.0)*0.4*diff*atmosColor)*sunset;  //distant fog.
#else // atmosphere-less planetoids and dim stars
	frag_color =
		material.emission +
#ifdef TERRAIN_WITH_LAVA
		varyingEmission +
#endif
		(scene.ambient * color) +
		(diff * final * 2.0);
#endif //ATMOSPHERE

#else // NUM_LIGHTS > 0 -- unlit rendering - stars
	vec4 color = vec4(1.0, 1.0, 1.0, 1.0);
	//emission is used to boost colour of stars, which is a bit odd
	frag_color = material.emission + color;
#endif
	SetFragDepth();
}
