// Copyright © 2008-2026 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#include "attributes.glsl"
#include "lib.glsl"
#include "basesphere_uniforms.glsl"

#define WATER_SHINE 16.0

uniform sampler2D texture0;
uniform sampler2D texture1;
in vec2 texCoord0;

in float dist;

in vec3 varyingEyepos;
in vec3 varyingNormal;
in vec4 vertexColor;

out vec4 frag_color;

uniform float PatchDetailFrequency;

#define detailScaleHi (PatchDetailFrequency * 4.0)
#define detailScaleLo (PatchDetailFrequency * 0.5)

void main(void)
{
	vec4 hidetail = texture(texture0, texCoord0 * detailScaleHi);
	vec4 lodetail = texture(texture1, texCoord0 * detailScaleLo);

	// calculate the detail texture contribution from hi and lo textures
	float hiloMix = exp(-0.004 * dist);
	float detailMix = exp(-0.001 * dist);
	vec4 detailVal = mix(lodetail, hidetail, hiloMix);
	vec4 detailMul = mix(vec4(1.0), detailVal, detailMix);

	// Use the detail value to multiply the final colour before lighting
	vec4 ambient = scene.ambient * vertexColor * 0.f;
	vec4 final = detailMul * vertexColor;

#ifdef ATMOSPHERE
	final = ambient + final;
#else
	// add extra brightness to atmosphere-less planetoids and dim stars
	final = ambient + final * 2.0;
#endif

	frag_color = final;

	frag_color = toSRGB(1 - exp(-frag_color));
}
