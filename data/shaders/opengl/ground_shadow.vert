// Copyright © 2008-2026 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#include "attributes.glsl"
#include "lib.glsl"

uniform vec3 planeNormal;
uniform float planeD;
uniform vec3 projectDir;

void main(void)
{
	vec4 viewPos4 = uViewMatrix * a_vertex;
	vec3 viewPos = viewPos4.xyz;

	float denom = dot(planeNormal, projectDir);
	if (abs(denom) > 1e-6) {
		float t = (planeD - dot(planeNormal, viewPos)) / denom;
		viewPos = viewPos + projectDir * t;
	}

	// Nudge above the ground plane to reduce depth fighting with terrain.
	viewPos += planeNormal * 0.05;

	// We transformed into view space to calculate the shadow vertices, now
	// transform back before applying the standard projection matrix
	gl_Position = uViewProjectionMatrix * inverse(uViewMatrix) * vec4(viewPos, 1.0);
}
