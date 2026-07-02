// Copyright © 2008-2026 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#include "attributes.glsl"
#include "lib.glsl"

uniform float shadowStrength;

out vec4 frag_color;

void main(void)
{
	// Get the view-space depth for this shadow fragment and store it in the shadow texture R channel
	// Shadow strength accumulates in the G channel

	float shadowZ = gl_FragCoord.z;

	// R = view space shadow distance, G = shadow strength.
	frag_color = vec4(shadowZ, shadowStrength, 0.0, 1.0);
}
