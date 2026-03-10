// Copyright © 2008-2026 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

// #extension GL_ARB_gpu_shader5 : enable

#include "attributes.glsl"
#include "lib.glsl"

out vec2 texCoord0;
out vec3 face_norm;
out vec4 flame_color;
out float ramp;

float map_percentage( float val, float base_percent)
{
   return base_percent + val * (1.0 - base_percent);
}

void main(void)
{
	// emission.r is the thruster power setting
	float thruster_power = material.emission.r;
	// emission.a is the random flicker value
	float random = material.emission.a;

	// power setting effects the brightness and opacity of the flame
	float brightness = map_percentage(thruster_power, 0.35);
	// flicker should only change the brightness a small amount each frame
	float flicker = map_percentage(random, 0.95);
	float flicker_len = map_percentage(1 - random, 0.90);

	flame_color = material.diffuse * flicker * brightness;
	// a very light blue flickering glow around flame
	flame_color.rg *= vec2(flicker);

	float flame_width = map_percentage(thruster_power, 0.8) * flicker;
	// the power setting also controls the length of the flame
	// flame length can range from 20% to 100%
	// random flicker can adjust the flame length by up to 20%
	float flame_length = map_percentage(thruster_power, 0.2) * flicker_len;
	// modulate the flame shape using flicker
	// flame_length only effects the z axis which is the thrust direction
	vec4 scale = vec4(flame_width, flame_width, flame_length, 1.0);

	gl_Position = uViewProjectionMatrix * (a_vertex * scale);

	face_norm = normalize(normalMatrix() * a_normal);

	texCoord0 = a_uv0.xy;
	// set the blend ramp
	ramp = a_uv0.y * 0.5;
}
