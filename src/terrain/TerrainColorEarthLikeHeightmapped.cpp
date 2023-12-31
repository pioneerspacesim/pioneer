// Copyright © 2008-2024 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#include "Terrain.h"
#include "TerrainNoise.h"

using namespace TerrainNoise;

template <>
const char *TerrainColorFractal<TerrainColorEarthLikeHeightmapped>::GetColorFractalName() const { return "EarthLikeHeightmapped"; }

template <>
TerrainColorFractal<TerrainColorEarthLikeHeightmapped>::TerrainColorFractal(const SystemBody *body) :
	Terrain(body)
{
	// crappy water
	//double height = m_maxHeightInMeters*0.5;
	//SetFracDef(3, m_maxHeightInMeters, 1e8, 50.0);
	//SetFracDef(2, m_maxHeightInMeters, 10, 10.0);
	m_surfaceEffects |= Terrain::EFFECT_WATER;
}

template <>
vector3d TerrainColorFractal<TerrainColorEarthLikeHeightmapped>::GetColor(const vector3d &p, double height, const vector3d &norm) const
{
	double n = m_invMaxHeight * height;
	double flatness = pow(p.Dot(norm), 8.0);

	double equatorial_desert = (2.0 - m_icyness) * (-1.0 + 2.0 * octavenoise(12, 0.5, 2.0, (n * 2.0) * p)) *
		1.0 * (2.0 - m_icyness) * (1.0 - p.y * p.y);
	vector3d color_cliffs = m_darkrockColor[5];
	vector3d col, tex1, tex2;

	if (n > 0) {
		// ice on mountains
		if (flatness > 0.6 / Clamp(n * m_icyness + (m_icyness * 0.5) + (fabs(p.y * p.y * p.y * 0.38)), 0.1, 1.0)) {
			col = interpolate_color(terrain_colournoise_rock, color_cliffs, m_rockColor[5]);
			col = interpolate_color(flatness, col, vector3d(1, 1, 1));
			return col;
		}
		//polar ice-caps
		if ((m_icyness * 0.5) + (fabs(p.y * p.y * p.y * 0.38)) > 0.6) {
			//if (flatness > 0.5/Clamp(fabs(p.y*m_icyness), 0.1, 1.0)) {
			col = interpolate_color(terrain_colournoise_rock, color_cliffs, m_rockColor[5]);
			col = interpolate_color(flatness, col, vector3d(1, 1, 1));
			return col;
		}
	}

	// water
	if (n <= 0) {
		// waves
		n += terrain_colournoise_water;
		n *= 0.1;
		col = interpolate_color(equatorial_desert, vector3d(0, 0, 0.15), vector3d(0, 0, 0.25));
		col = interpolate_color(n, col, vector3d(0, 0.8, 0.6));
		return col;
	}
	flatness = pow(p.Dot(norm), 16.0);
	// More sensitive height detection for application of colours
	if (n > 0.5) {
		n -= 0.5;
		n *= 2.0;
		color_cliffs = interpolate_color(n, m_darkrockColor[2], m_rockColor[4]);
		col = interpolate_color(equatorial_desert, m_rockColor[2], m_rockColor[4]);
		col = interpolate_color(n, col, m_darkrockColor[6]);
		tex1 = interpolate_color(terrain_colournoise_rock, col, color_cliffs);
		tex2 = interpolate_color(terrain_colournoise_sand, col, m_darkdirtColor[3]);
		col = interpolate_color(flatness, tex1, tex2);
		return col;
	} else if (n > 0.25) {
		n -= 0.25;
		n *= 4.0;
		color_cliffs = interpolate_color(n, m_rockColor[3], m_darkplantColor[4]);
		col = interpolate_color(equatorial_desert, m_darkrockColor[3], m_darksandColor[1]);
		col = interpolate_color(n, col, m_rockColor[2]);
		tex1 = interpolate_color(terrain_colournoise_rock, col, color_cliffs);
		tex2 = interpolate_color(terrain_colournoise_sand, col, m_darkdirtColor[3]);
		col = interpolate_color(flatness, tex1, tex2);
		return col;
	} else if (n > 0.05) {
		n -= 0.05;
		n *= 5.0;
		color_cliffs = interpolate_color(equatorial_desert, m_darkrockColor[5], m_darksandColor[7]);
		col = interpolate_color(equatorial_desert, m_darkplantColor[2], m_sandColor[2]);
		col = interpolate_color(n, col, m_darkrockColor[3]);
		tex1 = interpolate_color(terrain_colournoise_rock, col, color_cliffs);
		tex2 = interpolate_color(terrain_colournoise_forest, col, color_cliffs);
		col = interpolate_color(flatness, tex1, tex2);
		return col;
	} else if (n > 0.01) {
		n -= 0.01;
		n *= 25.0;
		color_cliffs = m_darkdirtColor[7];
		col = interpolate_color(equatorial_desert, m_plantColor[1], m_plantColor[0]);
		col = interpolate_color(n, col, m_darkplantColor[2]);
		tex1 = interpolate_color(terrain_colournoise_rock, col, color_cliffs);
		tex2 = interpolate_color(terrain_colournoise_grass, color_cliffs, col);
		col = interpolate_color(flatness, tex1, tex2);
		return col;
	} else if (n > 0.005) {
		n -= 0.005;
		n *= 200.0;
		color_cliffs = m_dirtColor[2];
		col = interpolate_color(equatorial_desert, m_darkplantColor[0], m_sandColor[1]);
		col = interpolate_color(n, col, m_plantColor[0]);
		tex1 = interpolate_color(terrain_colournoise_rock, col, color_cliffs);
		tex2 = interpolate_color(terrain_colournoise_grass, color_cliffs, col);
		col = interpolate_color(flatness, tex1, tex2);
		return col;
	} else {
		n *= 200.0;
		color_cliffs = m_darksandColor[0];
		col = interpolate_color(equatorial_desert, m_sandColor[0], m_sandColor[1]);
		col = interpolate_color(n, col, m_darkplantColor[0]);
		tex1 = interpolate_color(terrain_colournoise_rock, col, color_cliffs);
		tex2 = interpolate_color(terrain_colournoise_sand, col, color_cliffs);
		return col = interpolate_color(flatness, tex1, tex2);
	}
}
