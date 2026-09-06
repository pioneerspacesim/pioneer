// Copyright © 2008-2026 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#include "attributes.glsl"
#include "lib.glsl"
#include "basesphere_uniforms.glsl"
#include "rayleigh-lib.glsl"

uniform int NumShadows;
uniform sampler2D densityLUT;
uniform sampler2D scatterLUT;

out vec3 varyingEyepos;
out vec3 varyingNormal;
out vec4 vertexColor;

out vec2 texCoord0;
out float dist;

void main(void)
{
	gl_Position = matrixTransform();
	vertexColor = a_color;
	varyingEyepos = vec3(uViewMatrix * a_vertex);
	varyingNormal = normalize(normalMatrix() * a_normal);

	texCoord0 = a_uv0.xy;
	dist = length(varyingEyepos);

	vec3 eyeposScaled = varyingEyepos * geosphereInvRadius;
	vec3 eyenorm = normalize(varyingEyepos);
	vec3 tnorm = normalize(varyingNormal);
	vec4 diff = vec4(0.0);

#if (NUM_LIGHTS > 0)
	vec3 P = eyeposScaled - geosphereCenter;
	vec3 V = normalize(P);

	float AU = 149598000000.0;

	// coordinates, in planet radius
	vec4 planet = vec4(geosphereCenter, geosphereRadius);
	vec4 atmosphere = vec4(geosphereCenter, geosphereAtmosTopRad);

	vec4 terrainColor = vec4(0.f);
	vec4 atmosphereColor = vec4(0.f);

	for (int i=0; i<NUM_LIGHTS; ++i) {
		vec3 terrainDiffIn = vec3(0.f);
		vec3 terrainDiffOut = vec3(0.f);
		vec3 atmosphereDiff = vec3(0.f);
		vec3 waterSpecular = vec3(0.f);

		vec3 L = normalize(uLight[i].position.xyz);
		float uneclipsed = clamp(calcUneclipsed(eclipse, NumShadows, V, L), 0.0, 1.0);
		CalcPlanetDiffuse(diff, uLight[i].diffuse, L, tnorm, uneclipsed);

		vec3 lightPosAU = uLight[i].position.xyz / AU;
		// Pioneer uses a LDR pipeline without automatic exposure control, so
		// inverse-square falloff would make most distant planets impossible to
		// see.
		float intensity = 1.f / length(lightPosAU);

		vec4 lightColor = toLinear(uLight[i].diffuse);

		// start with diffuse terrain color
		vec3 terrain = vertexColor.xyz * lightColor.xyz * uneclipsed * max(0.f, dot(L, tnorm)) * intensity;

		lightColor *= intensity;

		// TODO: this should compute inscattering + A/B transmittance and pass
		// as a varying to fragment shader so we can run Blinn-Phong lighting
		// per-pixel.
		//
		// There are two multiplications by optical transmittance, one for the
		// sun->surface ray and once for the surface->view ray; the optical
		// transmittance and inscattering can be computed for each and passed to
		// the fragment shader.

#ifdef ATMOSPHERE
		// Atmospheric outscattering along the ray between the light source to the point on terrain
		terrain *= calculateRayTransmittance(P, L, scatterLUT);

#ifdef TERRAIN_WITH_WATER
		//water only for specular
		if (vertexColor.b > 0.05 && vertexColor.r < 0.05) {
			// TODO: this could be replaced with calculateRayInscattering, but there is a slight visual difference. Need to track down the cause.
			waterSpecular = calculateAtmosphereColor(planet, atmosphere, lightColor, reflect(L, V), (P - geosphereCenter) * geosphereRadius, eyenorm, uneclipsed, densityLUT, scatterLUT);
		}
#endif // TERRAIN_WITH_WATER

		// Water has no diffuse reflection, and is fully specular.
		// TODO: this formulation is likely incorrect, this needs to be Blinn-Phong (N dot H) * (inscattering + direct light * transmittance)
		terrain = terrain * step(waterSpecular, vec3(0)) + waterSpecular * 20;
#endif // ATMOSPHERE

		// add lava glow
#ifdef TERRAIN_WITH_LAVA
		if ( vertexColor.r > 0.4 && vertexColor.g < 0.2 && vertexColor.b < 0.4 ) {
			terrain += 3.0 * vertexColor.xyz * (vertexColor.r + vertexColor.g + vertexColor.b);
		} else {
			terrain += material.emission.xyz;
		}
#endif // TERRAIN_WITH_LAVA

#ifdef ATMOSPHERE
		// some light is again lost in atmosphere

		vec2 opticalDistance = vec2(0);
		vec3 inscattering = calculateRayInscattering(opticalDistance, P, -eyenorm, dist * geosphereInvRadius, L, lightColor.xyz, densityLUT, scatterLUT);

		// Apply the optical transmittance of the terrain -> view vector to our accumulated light
		terrain *= calcTransmittance(opticalDistance);
		// And add the (pre-damped) inscattering over the same length. We multiply by 4 to fake multiple scattering.
		terrain += inscattering * 4;

#endif // ATMOSPHERE

		terrainColor.xyz += terrain;
	}

	vertexColor = terrainColor;
#endif // (NUM_LIGHTS > 0)
}
