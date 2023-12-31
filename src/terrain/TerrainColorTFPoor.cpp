// Copyright © 2008-2024 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#include "Terrain.h"
#include "TerrainNoise.h"

using namespace TerrainNoise;

template <>
const char *TerrainColorFractal<TerrainColorTFPoor>::GetColorFractalName() const { return "TFPoor"; }

template <>
TerrainColorFractal<TerrainColorTFPoor>::TerrainColorFractal(const SystemBody *body) :
	Terrain(body)
{
	m_surfaceEffects |= Terrain::EFFECT_WATER;
}

template <>
vector3d TerrainColorFractal<TerrainColorTFPoor>::GetColor(const vector3d &p, double height, const vector3d &norm) const
{
	double n = m_invMaxHeight * height;
	double flatness = pow(p.Dot(norm), 8.0);

	double continents = 0;
	double equatorial_desert = (2.0 - m_icyness) * (-1.0 + 2.0 * octavenoise(12, 0.5, 2.0, (n * 2.0) * p)) *
		1.0 * (2.0 - m_icyness) * (1.0 - p.y * p.y);
	vector3d color_cliffs = m_darkrockColor[5];
	vector3d col, tex1, tex2;
	// ice on mountains and poles
	if (fabs(m_icyness * p.y) + m_icyness * n > 1) {
		col = interpolate_color(terrain_colournoise_rock2, color_cliffs, vector3d(.9, .9, .9));
		col = interpolate_color(flatness, col, vector3d(1, 1, 1));
		return col;
	}
	//we don't want water on the poles if there are ice-caps
	if (fabs(m_icyness * p.y) > 0.67) {
		col = interpolate_color(equatorial_desert, m_sandColor[2], m_darksandColor[5]);
		col = interpolate_color(flatness, col, vector3d(1, 1, 1));
		return col;
	}
	// This is for fake ocean depth by the coast.
	continents = ridged_octavenoise(GetFracDef(3), 0.55, p) * (1.0 - m_sealevel) - ((m_sealevel * 0.1) - 0.1);

	// water
	if (n <= 0) {
		// Oooh, pretty coastal regions with shading based on underwater depth.
		n += continents; // - (GetFracDef(3).amplitude*m_sealevel*0.49);
		n *= n * 10.0;
		//n = (n>0.3 ? 0.3-(n*n*n-0.027) : n);
		col = interpolate_color(n, vector3d(0, 0.0, 0.1), vector3d(0, 0.5, 0.5));
		return col;
	}
	// More sensitive height detection for application of colours
	if (n > 0.5) {
		n -= 0.5;
		n *= 2.0;
		//color_cliffs = m_rockColor[1];
		col = interpolate_color(equatorial_desert, m_rockColor[2], m_rockColor[6]);
		col = interpolate_color(n, col, m_darkrockColor[6]);
		tex1 = interpolate_color(terrain_colournoise_rock, col, color_cliffs);
		tex2 = interpolate_color(terrain_colournoise_rock2, col, color_cliffs);
		col = interpolate_color(flatness, tex1, tex2);
		return col;
	} else if (n > 0.25) {
		n -= 0.25;
		n *= 4.0;
		color_cliffs = m_rockColor[3];
		col = interpolate_color(equatorial_desert, m_darkrockColor[4], m_darksandColor[6]);
		col = interpolate_color(n, col, m_rockColor[2]);
		tex1 = interpolate_color(terrain_colournoise_rock, col, color_cliffs);
		tex2 = interpolate_color(terrain_colournoise_mud, col, color_cliffs);
		col = interpolate_color(flatness, tex1, tex2);
		return col;
	} else if (n > 0.05) {
		n -= 0.05;
		n *= 5.0;
		col = interpolate_color(equatorial_desert, m_darkrockColor[5], m_darksandColor[7]);
		color_cliffs = col;
		col = interpolate_color(equatorial_desert, m_darksandColor[2], m_sandColor[2]);
		col = interpolate_color(n, col, m_darkrockColor[3]);
		tex1 = interpolate_color(terrain_colournoise_mud, col, color_cliffs);
		tex2 = interpolate_color(terrain_colournoise_grass, col, color_cliffs);
		col = interpolate_color(flatness, tex1, tex2);
		return col;
	} else if (n > 0.01) {
		n -= 0.01;
		n *= 25.0;
		color_cliffs = m_darkplantColor[0];
		col = interpolate_color(equatorial_desert, m_sandColor[1], m_sandColor[0]);
		col = interpolate_color(n, col, m_darksandColor[2]);
		tex1 = interpolate_color(terrain_colournoise_grass, col, color_cliffs);
		tex2 = interpolate_color(terrain_colournoise_grass2, col, color_cliffs);
		col = interpolate_color(flatness, tex1, tex2);
		return col;
	} else if (n > 0.005) {
		n -= 0.005;
		n *= 200.0;
		color_cliffs = m_plantColor[0];
		col = interpolate_color(equatorial_desert, m_darkplantColor[0], m_sandColor[1]);
		col = interpolate_color(n, col, m_plantColor[0]);
		tex1 = interpolate_color(terrain_colournoise_sand2, col, color_cliffs);
		tex2 = interpolate_color(terrain_colournoise_grass, col, color_cliffs);
		col = interpolate_color(flatness, tex1, tex2);
		return col;
	} else {
		n *= 200.0;
		color_cliffs = m_darksandColor[0];
		col = interpolate_color(equatorial_desert, m_sandColor[0], m_sandColor[1]);
		col = interpolate_color(n, col, m_darkplantColor[0]);
		tex1 = interpolate_color(terrain_colournoise_sand, col, color_cliffs);
		//tex2 = interpolate_color(terrain_colournoise_sand2, col, color_cliffs);
		col = interpolate_color(flatness, tex1, col);
		return col;
	}
}
