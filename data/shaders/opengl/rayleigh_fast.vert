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
	vec3 eyenorm = normalize(varyingEyepos.xyz);
	vec3 specularHighlight = vec3(0.0);

	vec2 skyDist = raySphereIntersect(geosphereCenter, eyenorm, geosphereAtmosTopRad);

#if (NUM_LIGHTS > 0)
	for (int i=0; i<NUM_LIGHTS; ++i) {
		vec3 lightDir = normalize(vec3(uLight[i].position));

		vec3 sphereCenter = geosphereCenter * geosphereRadius;
		specularHighlight += computeIncidentLight(lightDir, eyenorm, sphereCenter, skyDist) * INV_NUM_LIGHTS;
	}
#endif

	vertexColor.rgb = specularHighlight;
}
