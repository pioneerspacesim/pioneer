// Copyright © 2008-2026 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt
//
// Radial falloff from bright centre with streaks that push the colour transition outward.

#include "attributes.glsl"
#include "lib.glsl"

in vec3 varyingNormal;

uniform vec4 ReentryFlight;
uniform float ReentryTime;

out vec4 frag_color;

float hash11(float p)
{
	return fract(sin(dot(vec2(p, p), vec2(12.9898, 78.233))) * 43758.5453);
}

void main(void)
{
	vec3 surface_normal = normalize(varyingNormal);
	vec3 wind_direction = ReentryFlight.xyz;
	float wind_speed = ReentryFlight.w;

	float dot_wind = dot(surface_normal, wind_direction);
	if (dot_wind < 0.001)
		discard;

	// Geometric angle: cos(theta) = dot_wind, theta in [0, pi/2] on windward cap
	float theta = acos(dot_wind);

	// Azimuth phi around wind axis: project surface normal onto plane perpendicular to wind direction
	vec3 wref = (abs(wind_direction.y) < 0.9) ? vec3(0.0, 1.0, 0.0) : vec3(1.0, 0.0, 0.0);
	vec3 tangent = normalize(cross(wind_direction, wref));
	vec3 bitangent = cross(wind_direction, tangent);
	vec3 perpend = surface_normal - wind_direction * dot_wind;
	float phi = atan(dot(perpend, bitangent), dot(perpend, tangent));

	// Streak effect from phi (and time). ReentryTime < 0 means game is paused, so don't draw the streaks
	float time_hash = hash11(ReentryTime);
	float wob = sin(phi * 22.0 * wind_speed + time_hash * 2.0 * PI) * sin(phi * 5.34 * wind_speed - time_hash * 3.7 * PI + 13);
	float spike = pow(wob * 0.5 + 0.5, 80) * 0.1;
	if (ReentryTime < 0) spike = 0;

	// Combine two elements to get final color. Both depend on wind speed.

	float reduced_dot_wind = (dot_wind - 0.4) * (1.0 / 0.6);

	// Base glow - starts off with a small amount of one colour at the nose, then gets wider and
	// with more of the second colour as wind speed increases
	float intensity = reduced_dot_wind * wind_speed;

	// Occasional streaks - get more frequent as wind speed increases
	intensity += spike * wind_speed * (reduced_dot_wind * 0.5 + 0.5);

	intensity = min(intensity, wind_speed);

	float color_intensity = max(0, (intensity - 0.3) * 3.0);
	float alpha_intensity = clamp((intensity - 0.1) * 3.0, 0, 0.95);

	vec3 start_col = vec3(0.3, 0.4, 1.0);
	vec3 end_col = vec3(0.5, 0.25, 0.5);

	vec3 rgb = start_col + (end_col * color_intensity);
	frag_color = vec4(rgb, alpha_intensity);
}
