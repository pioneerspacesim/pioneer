// Copyright © 2008-2026 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#include "attributes.glsl"
#include "lib.glsl"
#include "basesphere_uniforms.glsl"
#include "rayleigh.glsl"

uniform int NumShadows;
uniform sampler2D scatterLUT;
uniform sampler2D rayleighLUT;
uniform sampler2D mieLUT;

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
	// coordinates, in planet radius
	vec4 planet = vec4(geosphereCenter, geosphereRadius);
	vec4 atmosphere = vec4(geosphereCenter, geosphereAtmosTopRad);

	for (int i=0; i<NUM_LIGHTS; ++i) {
		vec3 lightDir = normalize(vec3(uLight[i].position));

		float uneclipsed = clamp(calcUneclipsedSky(eclipse, NumShadows, a, b, lightDir), 0.0, 1.0);

		vec3 lightPosAU = uLight[i].position.xyz / AU;
		float intensity = 1.f / dot(lightPosAU, lightPosAU); // magic to avoid calculating length and then squaring it

		specularHighlight += calculateAtmosphereColor(planet, atmosphere, toLinear(uLight[i].diffuse), lightDir, vec3(0.0), eyenorm, uneclipsed, scatterLUT) * intensity;
	}
#endif

	vec4 color = vec4(specularHighlight.rgb, 1.0) * 20;

	frag_color = toSRGB(1 - exp(-color));

#if 1
    frag_color = vec4(0.f, 0.f, 0.f, 1.f);

    // gl_FragCoord: [0-1280], [0, 720]

    frag_color.x = texelFetch(rayleighLUT, ivec2(gl_FragCoord.x - 512, gl_FragCoord.y - 232), 0).x;
    //frag_color.y = texture(mieLUT, vec2(gl_FragCoord.x / (64 * 2), gl_FragCoord.y / (36 * 256))).x;
    // draw border
    bool outer_range = (136 < gl_FragCoord.x) || (620 < gl_FragCoord.y);
    bool inner_range = (gl_FragCoord.x < 128) && (gl_FragCoord.y < 612);

    if (!inner_range && !outer_range) {
        frag_color.xyz = vec3(0.f, 0.f, 0.25f);
    }

    //frag_color.xyz = texelFetch(scatterLUT, ivec2(gl_FragCoord.x / 64, gl_FragCoord.y / 36), 0).x == 0 ? vec3(0.1f, 0.0f, 0.1f) : vec3(0.0f, 0.0f, 0.1f);
#endif
}
