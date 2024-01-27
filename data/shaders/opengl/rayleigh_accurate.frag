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
	vec3 eyenorm = normalize(varyingEyepos.xyz);
	vec3 specularHighlight = vec3(0.0);

    vec2 atmosDist  = raySphereIntersect(geosphereCenter, eyenorm, geosphereAtmosTopRad);
	// Invalid ray, skip shading this pixel
	// (can improve performance when spatially coherent)
	if (atmosDist.x == 0.0 && atmosDist.y == 0.0) {
		frag_color = vec4(0.0);
		return;
	}

	// a&b scaled so length of 1.0 means planet surface.
	vec3 a = atmosDist.x * eyenorm - geosphereCenter;
	vec3 b = atmosDist.y * eyenorm - geosphereCenter;

	float AU = 149598000000.0;

#if (NUM_LIGHTS > 0)
	for (int i=0; i<NUM_LIGHTS; ++i) {
		vec3 lightDir = normalize(vec3(uLight[i].position));

		float uneclipsed = clamp(calcUneclipsedSky(eclipse, NumShadows, a, b, lightDir), 0.0, 1.0);

		// Convert from radius-relative to real coordinates
		vec3 center = geosphereCenter * geosphereRadius;

		vec3 lightPosAU = uLight[i].position.xyz / AU;
		float intensity = 1.f / dot(lightPosAU, lightPosAU); // magic to avoid calculating length and then squaring it

		specularHighlight += computeIncidentLight(lightDir, eyenorm, center, atmosDist, toLinear(uLight[i].diffuse), uneclipsed) * intensity;
	}
#endif

	vec4 color = vec4(specularHighlight.rgb, 1.0) * 20;

	frag_color = toSRGB(1 - exp(-color));

}
