// Copyright © 2008-2023 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#include "attributes.glsl"
#include "lib.glsl"
#include "basesphere_uniforms.glsl"

uniform int NumShadows;

in vec4 varyingEyepos;

out vec4 frag_color;

void sphereEntryExitDist(out float near, out float far, const in vec3 sphereCenter, const in vec3 eyeTo, const in float radius)
{
	vec3 v = -sphereCenter;
	vec3 dir = normalize(eyeTo);
	float b = -dot(v, dir);
	float det = (b * b) - dot(v, v) + (radius * radius);
	float i1, i2;
	near = 0.0;
	far = 0.0;
	if (det > 0.0) {
		det = sqrt(det);
		i1 = b - det;
		i2 = b + det;
		if (i2 > 0.0) {
			near = max(i1, 0.0);
			far = i2;
		}
	}
}

void main(void)
{
	float skyNear, skyFar;
	vec3 eyenorm = normalize(varyingEyepos.xyz);
	float specularHighlight=0.0;

	sphereEntryExitDist(skyNear, skyFar, geosphereCenter, varyingEyepos.xyz, geosphereRadius * geosphereAtmosTopRad);
	float atmosDist = (skyFar - skyNear);
	float ldprod=0.0;

	// a&b scaled so length of 1.0 means planet surface.
	vec3 a = (skyNear * eyenorm - geosphereCenter) * geosphereInvRadius;
	vec3 b = (skyFar * eyenorm - geosphereCenter) * geosphereInvRadius;
	ldprod = AtmosLengthDensityProduct(a, b, atmosColor.a * geosphereAtmosFogDensity, atmosDist, geosphereAtmosInvScaleHeight);

	float fogFactor = 1.0 / exp(ldprod);
	vec4 atmosDiffuse = vec4(0.0);

#if (NUM_LIGHTS > 0)
	vec3 surfaceNorm = normalize(skyNear * eyenorm - geosphereCenter);
	for (int i=0; i<NUM_LIGHTS; ++i) {

		vec3 lightDir = normalize(vec3(uLight[i].position));

		float uneclipsed = clamp(calcUneclipsedSky(eclipse, NumShadows, a, b, lightDir), 0.0, 1.0);

		float nDotVP =  max(0.0, dot(surfaceNorm, lightDir));
		float nnDotVP = max(0.0, dot(surfaceNorm, -lightDir));  //need backlight to increase horizon
		atmosDiffuse +=  uLight[i].diffuse * uneclipsed * 0.5*(nDotVP+0.5*clamp(1.0-nnDotVP*4.0,0.0,1.0) * INV_NUM_LIGHTS);

		//Calculate Specular Highlight
		vec3 L = normalize(uLight[i].position.xyz - varyingEyepos.xyz);
		vec3 E = normalize(-varyingEyepos.xyz);
		vec3 R = normalize(-reflect(L,vec3(0.0)));
		specularHighlight += pow(max(dot(R,E),0.0),64.0) * uneclipsed * INV_NUM_LIGHTS;

	}
#endif

	//calculate sunset tone red when passing through more atmosphere, clamp everything.
	float atmpower = (atmosDiffuse.r+atmosDiffuse.g+atmosDiffuse.b)/3.0;
	vec4 sunset = vec4(0.8,clamp(pow(atmpower,0.8),0.0,1.0),clamp(pow(atmpower,1.2),0.0,1.0),1.0);

	atmosDiffuse.a = 1.0;
	frag_color = (1.0-fogFactor) * (atmosDiffuse*
		vec4(atmosColor.rgb, 1.0)) +
		(0.02-clamp(fogFactor,0.0,0.01))*atmosDiffuse*ldprod*sunset +     //increase light on lower atmosphere.
		atmosColor*specularHighlight*(1.0-fogFactor)*sunset;		  //add light from specularHighlight.
}
