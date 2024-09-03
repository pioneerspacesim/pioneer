// Copyright © 2008-2024 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#include "attributes.glsl"
#include "lib.glsl"
#include "basesphere_uniforms.glsl"

uniform int NumShadows;

in vec4 varyingEyepos;

out vec4 frag_color;

void main(void)
{
	vec3 eyenorm = normalize(varyingEyepos.xyz);
	float specularHighlight=0.0;

	vec2 viewDist = raySphereIntersect(geosphereCenter, eyenorm, geosphereAtmosTopRad);
	vec2 isect = raySphereIntersect(geosphereCenter, eyenorm, 1.0);

	float atmosDist = (viewDist.y - viewDist.x) * geosphereRadius;
	float ldprod=0.0;

	// a&b scaled so length of 1.0 means planet surface.
	vec3 a = viewDist.x * eyenorm - geosphereCenter;
	vec3 b = viewDist.y * eyenorm - geosphereCenter;
	ldprod = AtmosLengthDensityProduct(a, b, atmosColor.a * geosphereAtmosFogDensity, atmosDist, geosphereAtmosInvScaleHeight);

	float fogFactor = 1.0 / exp(ldprod);
	vec4 atmosDiffuse = vec4(0.0);

#if (NUM_LIGHTS > 0)
	vec3 surfaceNorm = normalize(viewDist.x * eyenorm - geosphereCenter);
	for (int i=0; i<NUM_LIGHTS; ++i) {

		vec3 L = normalize(vec3(uLight[i].position));
		float uneclipsed = clamp(calcUneclipsedSky(eclipse, NumShadows, a, b, L), 0.0, 1.0);

		CalcPlanetDiffuse(atmosDiffuse, toLinear(uLight[i].diffuse), L, surfaceNorm, uneclipsed);

		// Calculate Specular Highlight (halo around the light source)
		specularHighlight += pow(max(dot(L, eyenorm), 0.0), 64.0) * uneclipsed * INV_NUM_LIGHTS;

	}
#endif

	// Tonemap in sRGB space to match existing visuals
	atmosDiffuse = toSRGB(atmosDiffuse);
	atmosDiffuse = 1.0 - exp(-atmosDiffuse);

	//calculate sunset tone red when passing through more atmosphere, clamp everything.
	float atmpower = (atmosDiffuse.r+atmosDiffuse.g+atmosDiffuse.b)/3.0;
	vec4 sunset = vec4(0.8,clamp(pow(atmpower,0.8),0.0,1.0),clamp(pow(atmpower,1.2),0.0,1.0),1.0);

	atmosDiffuse.a = 1.0;
	frag_color = (1.0-fogFactor) * (atmosDiffuse*
		vec4(atmosColor.rgb, 1.0)) +
		(0.02-clamp(fogFactor,0.0,0.01))*atmosDiffuse*ldprod*sunset +     //increase light on lower atmosphere.
		atmosColor*specularHighlight*(1.0-fogFactor)*sunset;		  //add light from specularHighlight.
}
