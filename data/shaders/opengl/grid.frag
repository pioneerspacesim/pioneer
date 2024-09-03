// Copyright © 2008-2024 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

// Adapted from https://ourmachinery.com/post/borderland-between-rendering-and-editor-part-1/
// Credit to Tobias Persson for providing an example implementation.

#include "attributes.glsl"

layout(std140) uniform GridData {
	vec4 color_lod0;
	vec4 color_lod1;
	vec4 grid_info;
	vec2 grid_size;
	float line_width_px;
};

in vec3 v_eyepos;
in vec2 v_uv0;

out vec4 frag_color;

float log10(float v)
{
	return log(v) / log(10.0);
}

// Calculate the combined XY line coverage values for the given pixel with the defined grid setting
float calc_grid_coverage(vec2 uv, vec2 line_width, float cell_size)
{
	vec2 cover = 1 - abs(clamp(mod(uv + line_width * 0.5, cell_size) / line_width, 0, 1) * 2 - 1);
	return max(cover.x, cover.y);
}

void main(void)
{
	// Calculate the screen-space derivatives of the UV coordinates
	vec4 derivatives = vec4(dFdx(v_uv0), dFdy(v_uv0));
	vec2 dudv = vec2(length(derivatives.xz), length(derivatives.yw));

	vec3 grid_dir = grid_info.xyz;
	float cell_size = grid_info.w;

	float lod_level = max(0, log10(length(dudv) / cell_size) + 1);
	float lod_transition = fract(lod_level);

	// Calculate the grid cell size at the minimum visible LOD level and the next larger
	float lod0_cell_size = cell_size * pow(10, floor(lod_level));
	float lod1_cell_size = lod0_cell_size * 10;
	float lod2_cell_size = lod1_cell_size * 10;

	// Calculate the XY line coverage values for this pixel
	vec2 line_width = dudv * line_width_px; // size of a grid line in grid units
	float lod0_a = calc_grid_coverage(v_uv0, line_width, lod0_cell_size);
	float lod1_a = calc_grid_coverage(v_uv0, line_width, lod1_cell_size);
	float lod2_a = calc_grid_coverage(v_uv0, line_width, lod2_cell_size);

	// Generate all three separate color regions: bright, transition, dark
	vec4 color = lod2_a > 0 ? color_lod1 : lod1_a > 0 ? mix(color_lod1, color_lod0, lod_transition) : color_lod0;
	color.a *= lod2_a > 0 ? lod2_a : lod1_a > 0 ? lod1_a : lod0_a * (1.0 - lod_transition);

	// Smoothly fade out the very edges of the grid
	vec2 grid_edge = abs(v_uv0 / grid_size);
	color.a *= 1.0 - pow(max(grid_edge.x, grid_edge.y), 16);

	// Make the grid transparent at grazing angles
	vec3 viewdir = normalize(v_eyepos);
	color.a *= 1.0 - pow(1.0 - abs(dot(viewdir, grid_dir)), 8);

	// Add some subtle depth-fade to further reduce moire effect
	float viewdist = length(v_eyepos);
	color.a *= viewdist < 10 ? min(1.0, pow(viewdist, 4)) : min(1.0, max(grid_size.x, grid_size.y) / viewdist);

	frag_color = color;
}
