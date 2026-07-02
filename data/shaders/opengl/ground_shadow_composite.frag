// Copyright © 2008-2026 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#include "attributes.glsl"
#include "lib.glsl"

uniform sampler2D texture0;
uniform float shadowAlpha;
uniform float depthBias;

in vec2 texCoord0;

out vec4 frag_color;

void main(void)
{
	ivec2 size = textureSize(texture0, 0);
	ivec2 coord = clamp(ivec2(texCoord0 * vec2(size)), ivec2(0), size - ivec2(1));

	vec4 shadowData = texelFetch(texture0, coord, 0);
	float shadowZ = shadowData.r;
	float strength = shadowData.g;

	if (strength <= 0.001)
		discard;

	// Reverse-Z: larger depth values are closer. Bias the tested depth toward the
	// camera so the shadow wins over minor terrain bumps without shifting geometry.
	gl_FragDepth = shadowZ + depthBias;

	frag_color = vec4(0.0, 0.0, 0.0, strength * shadowAlpha);
}
