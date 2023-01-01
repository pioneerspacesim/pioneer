// Copyright © 2008-2023 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#include "attributes.glsl"
#include "lib.glsl"
#include "basesphere_uniforms.glsl"

in vec3 varyingEyepos;
in vec3 varyingNormal;
in vec3 varyingTexCoord0;

uniform samplerCube texture0; //diffuse

uniform int NumShadows;

out vec4 frag_color;

void main(void)
{
	vec3 eyepos = varyingEyepos;
	vec3 eyenorm = normalize(eyepos);
	vec3 tnorm = normalize(varyingNormal);
	vec4 diff = vec4(0.0);
	float nDotVP=0.0;
	float nnDotVP=0.0;

	vec3 v = (eyepos - geosphereCenter) * geosphereInvRadius;
	float lenInvSq = 1.0/(dot(v,v));
	for (int i=0; i<NUM_LIGHTS; ++i) {
		float uneclipsed = clamp(calcUneclipsed(eclipse, NumShadows, v, normalize(vec3(uLight[i].position))), 0.0, 1.0);
		nDotVP  = max(0.0, dot(tnorm, normalize(vec3(uLight[i].position))));
		nnDotVP = max(0.0, dot(tnorm, normalize(-vec3(uLight[i].position)))); //need backlight to increase horizon
		diff += uLight[i].diffuse * uneclipsed * 0.5*(nDotVP+0.5*clamp(1.0-nnDotVP*4.0,0.0,1.0) * INV_NUM_LIGHTS);
	}

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

	vec4 texColor = texture(texture0, varyingTexCoord0);

	frag_color =
		material.emission +
		fogFactor *
		((scene.ambient * texColor) +
		(diff * texColor)) +
		(1.0-fogFactor)*(diff*atmosColor) +
		  (0.02-clamp(fogFactor,0.0,0.01))*diff*ldprod*sunset +	      //increase fog scatter
		  (pow((1.0-pow(fogFactor,0.75)),256.0)*0.4*diff*atmosColor)*sunset;  //distant fog.
}
