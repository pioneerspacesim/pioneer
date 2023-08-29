// Copyright © 2008-2023 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#include "attributes.glsl"
#include "lib.glsl"
#include "basesphere_uniforms.glsl"
#include "rayleigh.glsl"

out vec4 varyingEyepos;
out vec4 vertexColor;

void main(void)
{
	gl_Position = matrixTransform();
	varyingEyepos = uViewMatrix * a_vertex;

    // compute incident light from each light source
	float skyNear, skyFar;
	vec3 eyenorm = normalize(varyingEyepos.xyz);
	vec3 specularHighlight = vec3(0.0);

	sphereEntryExitDist(skyNear, skyFar, geosphereCenter, varyingEyepos.xyz, geosphereRadius * geosphereAtmosTopRad);
#if (NUM_LIGHTS > 0)
	vec3 surfaceNorm = normalize(skyNear * eyenorm - geosphereCenter);
	for (int i=0; i<NUM_LIGHTS; ++i) {
		vec3 lightDir = normalize(vec3(uLight[i].position));

		specularHighlight += computeIncidentLight(lightDir, eyenorm, geosphereCenter, skyNear, skyFar) * INV_NUM_LIGHTS;
	}
#endif

	vertexColor.rgb = specularHighlight;
}
