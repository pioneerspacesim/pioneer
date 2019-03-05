// Copyright © 2008-2020 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#include "attributes.glsl"
#include "logz.glsl"
#include "lib.glsl"
#include "eclipse.glsl"

uniform vec4 atmosColor;
// to keep distances sane we do a nearer, smaller scam. this is how many times
// smaller the geosphere has been made
uniform float geosphereRadius;
uniform float geosphereInvRadius;
uniform float geosphereAtmosTopRad;
uniform vec3 geosphereCenter;
uniform float geosphereAtmosFogDensity;
uniform float geosphereAtmosInvScaleHeight;

uniform sampler2D texture0;	// hi detail
uniform sampler2D texture1;	// lo detail
uniform sampler2D texture2;	// lookup
uniform sampler2DArray texture3; // atlas
in vec2 uv0;
in vec2 uv1;

in float dist;
uniform float detailScaleHi;
uniform float detailScaleLo;

uniform Material material;
uniform Scene scene;

in vec3 varyingEyepos;
in vec3 varyingNormal;

#ifdef TERRAIN_WITH_LAVA
in vec4 varyingEmission;
#endif

out vec4 frag_color;

void main(void)
{
	vec4 hidetail = texture(texture0, uv0 * detailScaleHi);
	vec4 lodetail = texture(texture1, uv0 * detailScaleLo);

	vec3 eyepos = varyingEyepos;
	vec3 eyenorm = normalize(eyepos);
	vec3 tnorm = normalize(varyingNormal);
	vec4 diff = vec4(0.0);


	// calculate the detail texture contribution from hi and lo textures
	float hiloMix = exp(-0.004 * dist);
	float detailMix = exp(-0.001 * dist);
	vec4 detailVal = mix(lodetail, hidetail, hiloMix);
	vec4 detailMul = mix(vec4(1.0), detailVal, detailMix);

	float nDotVP=0.0;
	float nnDotVP=0.0;
#ifdef TERRAIN_WITH_WATER
	float specularReflection=0.0;
#endif

#ifdef TEXTURE0
	// LookupTexture & slope/height texture coords
	float terrainType = texture(texture2, uv1).r;
	// mul by 16 because values are 0..1 but we need them 0..15 for array indexing
	int id0 = int(terrainType * 16.0); 

	#if 1
	// cheap way of limiting the visual appearance of texture tiling
	// NB: this darkens the texture input
	vec4 s1 = texture(texture3, vec3(uv0 * detailScaleLo, float(id0))).rgba;
	vec4 s2 = texture(texture3, vec3(uv0 * detailScaleLo * -0.25, float(id0))).rgba;
	vec4 color = (s1 * s2) * 2.0;
	#else	
	vec4 color = texture(texture3, vec3(uv0 * detailScaleLo, float(id0))).rgba;
	#endif
#else
	vec4 color = vec4(0.313, 0.313, 0.313, 1.0);
#endif

#if (NUM_LIGHTS > 0)
	vec3 v = (eyepos - geosphereCenter) * geosphereInvRadius;
	float lenInvSq = 1.0/(dot(v,v));
	for (int i=0; i<NUM_LIGHTS; ++i) {
		float uneclipsed = clamp(calcUneclipsed(i, v, normalize(vec3(uLight[i].position))), 0.0, 1.0);
		nDotVP  = max(0.0, dot(tnorm, normalize(vec3(uLight[i].position))));
		nnDotVP = max(0.0, dot(tnorm, normalize(-vec3(uLight[i].position)))); //need backlight to increase horizon
		diff += uLight[i].diffuse * uneclipsed * 0.5*(nDotVP+0.5*clamp(1.0-nnDotVP*4.0,0.0,1.0) * INV_NUM_LIGHTS);

#ifdef TERRAIN_WITH_WATER
		//Specular reflection
		vec3 L = normalize(uLight[i].position.xyz - eyepos); 
		vec3 E = normalize(-eyepos);
		vec3 R = normalize(-reflect(L,tnorm)); 
		//water only for specular
	    if (color.b > 0.05 && color.r < 0.05) {
			specularReflection += pow(max(dot(R,E),0.0),16.0)*0.4 * INV_NUM_LIGHTS;
		}
#endif
	}

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
		vec3 a = (atmosStart * eyenorm - geosphereCenter) * geosphereInvRadius;
		vec3 b = (eyepos - geosphereCenter) * geosphereInvRadius;
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
	//emission is used to boost colour of stars, which is a bit odd
	frag_color = material.emission + color;
#endif
	SetFragDepth();
}
