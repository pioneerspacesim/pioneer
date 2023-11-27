// Copyright © 2008-2023 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#include "attributes.glsl"
#include "lib.glsl"
#include "basesphere_uniforms.glsl"
#include "rayleigh.glsl"

uniform int NumShadows;

in vec4 varyingEyepos;
in vec4 vertexColor;

out vec4 frag_color;

void main(void)
{
	float skyNear, skyFar;
	vec3 eyenorm = normalize(varyingEyepos.xyz);
	vec3 specularHighlight = vec3(0.0);

	float terrainNear, terrainFar;

	sphereEntryExitDist(skyNear, skyFar, geosphereCenter, varyingEyepos.xyz, geosphereRadius * geosphereAtmosTopRad);
	sphereEntryExitDist(terrainNear, terrainFar, geosphereCenter, varyingEyepos.xyz, geosphereRadius);

	// a&b scaled so length of 1.0 means planet surface.
	vec3 a = (skyNear * eyenorm - geosphereCenter) * geosphereInvRadius;
	vec3 b = (skyFar * eyenorm - geosphereCenter) * geosphereInvRadius;

	// far > 0.0 means intersection with planet surface
	// clip atmosphere ray against planet surface so we're not rendering "behind" geosphere
	if (terrainFar > 0.0) {
		skyFar = terrainNear;
		b = (terrainNear * eyenorm - geosphereCenter) * geosphereInvRadius;
	}

	vec4 atmosDiffuse = vec4(0.0);

#if (NUM_LIGHTS > 0)
	vec3 surfaceNorm = normalize(skyNear * eyenorm - geosphereCenter);
	for (int i=0; i<NUM_LIGHTS; ++i) {
		vec3 lightDir = normalize(vec3(uLight[i].position));

		float uneclipsed = clamp(calcUneclipsedSky(eclipse, NumShadows, a, b, lightDir), 0.0, 1.0);

		float nDotVP =  max(0.0, dot(surfaceNorm, lightDir));
		float nnDotVP = max(0.0, dot(surfaceNorm, -lightDir));  //need backlight to increase horizon
		atmosDiffuse +=  uLight[i].diffuse * uneclipsed * 0.5*(nDotVP+0.5*clamp(1.0-nnDotVP*4.0,0.0,1.0) * INV_NUM_LIGHTS);

		specularHighlight += computeIncidentLight(lightDir, eyenorm, geosphereCenter, skyNear, skyFar) * INV_NUM_LIGHTS;
	}
#endif

	atmosDiffuse.a = 1.0;
	vec4 color = atmosDiffuse *
		vec4(specularHighlight.rgb, 1.0) * 20;
	frag_color = (1 - exp(-color));
}
