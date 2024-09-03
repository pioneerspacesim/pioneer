// Copyright © 2008-2024 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

// Adapted from https://ourmachinery.com/post/borderland-between-rendering-and-editor-part-1/
// Credit to Tobias Persson for providing an example implementation.

#include "attributes.glsl"

layout(std140) uniform GridData {
	vec4 color_lod0;
	vec4 color_lod1;
	float line_spacing;
	float line_width_px;
};

in vec3 v_eyepos;
in vec3 v_normal;

out vec4 frag_color;

float log10(float v)
{
	return log(v) / log(10.0);
}

float calc_cell_lod(float cell_size, float lod_level)
{
	// The top LOD level highlights 90 degree increments, and lower LOD levels highlight 10, 1, 0.1 etc. degree increments
	return int(lod_level) == 1 ? cell_size * 9 : cell_size * pow(10.0, lod_level);
}

// Calculate the combined XY line coverage values for the given pixel with the defined grid setting
float calc_grid_coverage(vec2 uv, vec2 line_width, float cell_size)
{
	vec2 cover = 1 - abs(clamp(mod(uv + line_width * 0.5, cell_size) / line_width, 0, 1) * 2 - 1);
	return max(cover.x, cover.y);
}

void main(void)
{
	vec3 spherical = normalize((uViewMatrixInverse * vec4(v_eyepos, 1.0)).xyz);

	float theta = atan(spherical.z, spherical.x);
	float phi = acos(spherical.y);
	vec2 polar_coords = vec2(theta / PI, phi / PI);

	// Calculate the screen-space derivatives of the UV coordinates
	vec4 derivatives = vec4(dFdx(polar_coords), dFdy(polar_coords));
	vec2 dudv = vec2(length(derivatives.xz), length(derivatives.yw));

	// Each primary cell covers an area of 10 degrees and subdivides from there
	float cell_size = 1.0 / 18.0;

	float lod_level = min(1, log10(length(dudv) * line_spacing / cell_size) + 1);
	float lod_transition = fract(lod_level);

	// Calculate the grid cell size at the minimum visible LOD level and the next larger
	float lod0_cell_size = calc_cell_lod(cell_size, floor(lod_level));
	float lod1_cell_size = calc_cell_lod(cell_size, floor(lod_level) + 1);
	float lod2_cell_size = calc_cell_lod(cell_size, floor(lod_level) + 2);

	// Calculate the XY line coverage values for this pixel
	vec2 line_width = dudv * line_width_px; // size of a grid line in grid units
	float lod0_a = calc_grid_coverage(polar_coords, line_width, lod0_cell_size);
	float lod1_a = calc_grid_coverage(polar_coords, line_width, lod1_cell_size);
	float lod2_a = calc_grid_coverage(polar_coords, line_width, lod2_cell_size);

	// Generate all three separate color regions: bright, transition, dark
	vec4 color = lod2_a > 0 ? color_lod1 : lod1_a > 0 ? mix(color_lod1, color_lod0, lod_transition) : color_lod0;
	color.a *= max(lod2_a, max(lod1_a, lod0_a * (1.0 - lod_transition)));

	// Make the grid transparent at grazing angles
	vec3 viewdir = normalize(v_eyepos);
	color.a *= 1.0 - pow(1.0 - abs(dot(viewdir, v_normal)), 8);

	// Make the rear half of the grid sphere less visible
	color.a *= clamp(dot(viewdir, -v_normal) + 1.0, 0.2, 1.0);

	frag_color = color;
}
