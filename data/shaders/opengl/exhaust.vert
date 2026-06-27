// Copyright © 2008-2026 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#include "attributes.glsl"
#include "lib.glsl"

// Per-particle instance data
layout (location = 6) in vec4 a_instCenterSize;
layout (location = 7) in vec4 a_instJetVector;
layout (location = 8) in float a_instStretchScale;
layout (location = 9) in vec4 a_instColor;

out vec2 v_uv;
out vec4 v_color;

vec3 normalizedSafe(vec3 v)
{
	float lenSq = dot(v, v);
	if (lenSq < 1e-18)
		return vec3(1.0, 0.0, 0.0);
	return v * inversesqrt(lenSq);
}

void main(void)
{
	// Procedural unit quad: CCW triangle strip, lower-left origin (no per-vertex buffer).
	vec2 uv = vec2(float(gl_VertexID & 1), float(gl_VertexID >> 1));
	float ux = mix(-1.0, 1.0, uv.x);
	float vy = mix(-1.0, 1.0, uv.y);

	vec3 center = a_instCenterSize.xyz;
	float size = a_instCenterSize.w;
	vec3 jetVectorCam = a_instJetVector.xyz;
	float backboneCamZ = a_instJetVector.w;
	float stretchScale = a_instStretchScale;

	vec3 viewDir = normalizedSafe(-center);

	vec3 vProj = jetVectorCam - viewDir * dot(jetVectorCam, viewDir);
	vec3 axisV = normalizedSafe(vProj);
	vec3 axisU = normalizedSafe(cross(viewDir, axisV));

	float speedMag = dot(jetVectorCam, axisV);
	float attenuation = 1.0;
	float prevZ = abs(backboneCamZ - jetVectorCam.z);
	float curZ = abs(backboneCamZ);
	if ((prevZ > 0.1) && (curZ > 0.1))
		attenuation = min(1.0, curZ / prevZ);
	float stretch = stretchScale * speedMag * attenuation;

	vec3 worldPos = center + axisU * (ux * size) + axisV * (vy * size);

	// Bottom row of the quad is stretched back along the jet axis for streak particles.
	if (uv.y < 0.5)
		worldPos -= axisV * stretch;

	gl_Position = uViewProjectionMatrix * vec4(worldPos, 1.0);
	v_uv = uv;
	v_color = a_instColor;
}
