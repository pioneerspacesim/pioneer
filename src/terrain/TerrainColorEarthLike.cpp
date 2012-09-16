// Copyright © 2008-2012 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#include "Terrain.h"
#include "TerrainNoise.h"

using namespace TerrainNoise;

template <>
const char *TerrainColorFractal<TerrainColorEarthLike>::GetColorFractalName() const { return "EarthLike"; }

template <>
TerrainColorFractal<TerrainColorEarthLike>::TerrainColorFractal(const SystemBody *body) : Terrain(body)
{
	// crappy water
	//double height = m_maxHeightInMeters*0.5;
	//SetFracDef(3, m_maxHeightInMeters, 1e8, 50.0);
	//SetFracDef(2, m_maxHeightInMeters, 10, 10.0);
}

template <>
vector3d TerrainColorFractal<TerrainColorEarthLike>::GetColor(const vector3d &p, double height, const vector3d &norm)
{
	double n = m_invMaxHeight*height;
	double flatness = pow(p.Dot(norm), 8.0);

	double continents = 0;
	double equatorial_desert = (2.0-m_icyness)*(-1.0+2.0*octavenoise(12, 0.5, 2.0, (n*2.0)*p)) *
			1.0*(2.0-m_icyness)*(1.0-p.y*p.y);
	vector3d color_cliffs = m_darkrockColor[5];
	vector3d col, tex1, tex2;

	if (m_heightMap) {
		if (n > 0) {
			// ice on mountains
			if (flatness > 0.6/Clamp(n*m_icyness+(m_icyness*0.5)+(fabs(p.y*p.y*p.y*0.38)), 0.1, 1.0)) {
				if (textures) {
					col = interpolate_color(rock, color_cliffs, m_rockColor[5]);
					col = interpolate_color(flatness, col, vector3d(1,1,1));
				} else col = interpolate_color(flatness, color_cliffs, vector3d(1,1,1));
				return col;
			}
			//polar ice-caps
			if ((m_icyness*0.5)+(fabs(p.y*p.y*p.y*0.38)) > 0.6) {
				//if (flatness > 0.5/Clamp(fabs(p.y*m_icyness), 0.1, 1.0)) {
				if (textures) {
					col = interpolate_color(rock, color_cliffs, m_rockColor[5]);
					col = interpolate_color(flatness, col, vector3d(1,1,1));
				} else col = interpolate_color(flatness, color_cliffs, vector3d(1,1,1));
				return col;
			}
		}
	} else {
		// ice on mountains
		//printf("flatness : %d", flatness);
		if (flatness > 0.6/Clamp(n*m_icyness+(m_icyness*0.5)+(fabs(p.y*p.y*p.y*0.38)), 0.1, 1.0)) {
			if (textures) {
				col = interpolate_color(rock, color_cliffs, m_rockColor[5]);
				col = interpolate_color(flatness, col, vector3d(1,1,1));
			} else col = interpolate_color(flatness, color_cliffs, vector3d(1,1,1));
			return col;
		}
		//polar ice-caps
		if ((m_icyness*0.5)+(fabs(p.y*p.y*p.y*0.38)) > 0.6) {
			//if (flatness > 0.5/Clamp(fabs(p.y*m_icyness), 0.1, 1.0)) {
			if (textures) {
				col = interpolate_color(rock, color_cliffs, m_rockColor[5]);
				col = interpolate_color(flatness, col, vector3d(1,1,1));
			} else col = interpolate_color(flatness, color_cliffs, vector3d(1,1,1));
			return col;
		}
	}


	// This is for fake ocean depth by the coast.
		if (m_heightMap) {
			continents = 0;
		} else {
			continents = ridged_octavenoise(GetFracDef(3-m_fracnum), 0.55, p) * (1.0-m_sealevel) - ((m_sealevel*0.1)-0.1);
		}
	// water
	if (n <= 0) {
		if (m_heightMap) {
			// waves
			if (textures) {
				n += water;
				n *= 0.1;
			}
		} else {
		// Oooh, pretty coastal regions with shading based on underwater depth.
			n += continents;// - (GetFracDef(3).amplitude*m_sealevel*0.49);
			n *= n*10.0;
			//n = (n>0.3 ? 0.3-(n*n*n-0.027) : n);
		}
		col = interpolate_color(equatorial_desert, vector3d(0,0,0.15), vector3d(0,0,0.25));
		col = interpolate_color(n, col, vector3d(0,0.8,0.6));
		return col;
	}
	flatness = pow(p.Dot(norm), 16.0);
	// More sensitive height detection for application of colours
	if (n > 0.5) {
		n -= 0.5; n *= 2.0;
		color_cliffs = interpolate_color(n, m_darkrockColor[2], m_rockColor[4]);
		col = interpolate_color(equatorial_desert, m_rockColor[2], m_rockColor[4]);
		col = interpolate_color(n, col, m_darkrockColor[6]);
		if (textures) {
			tex1 = interpolate_color(rock, col, color_cliffs);
			tex2 = interpolate_color(sand, col, m_darkdirtColor[3]);
			col = interpolate_color(flatness, tex1, tex2);
		} else col = interpolate_color(flatness, color_cliffs, col);
		return col;
	}
	else if (n > 0.25) {
		n -= 0.25; n *= 4.0;
		color_cliffs = interpolate_color(n, m_rockColor[3], m_darkplantColor[4]);
		col = interpolate_color(equatorial_desert, m_darkrockColor[3], m_darksandColor[1]);
		col = interpolate_color(n, col, m_rockColor[2]);
		if (textures) {
			tex1 = interpolate_color(rock, col, color_cliffs);
			tex2 = interpolate_color(sand, col, m_darkdirtColor[3]);
			col = interpolate_color(flatness, tex1, tex2);
		} else col = interpolate_color(flatness, color_cliffs, col);
		return col;
	}
	else if (n > 0.05) {
		n -= 0.05; n *= 5.0;
		color_cliffs = interpolate_color(equatorial_desert, m_darkrockColor[5], m_darksandColor[7]);
		col = interpolate_color(equatorial_desert, m_darkplantColor[2], m_sandColor[2]);
		col = interpolate_color(n, col, m_darkrockColor[3]);
		if (textures) {
			tex1 = interpolate_color(rock, col, color_cliffs);
			tex2 = interpolate_color(forest, col, color_cliffs);
			col = interpolate_color(flatness, tex1, tex2);
		} else col = interpolate_color(flatness, color_cliffs, col);
		return col;
	}
	else if (n > 0.01) {
		n -= 0.01; n *= 25.0;
		color_cliffs = m_darkdirtColor[7];
		col = interpolate_color(equatorial_desert, m_plantColor[1], m_plantColor[0]);
		col = interpolate_color(n, col, m_darkplantColor[2]);
		if (textures) {
			tex1 = interpolate_color(rock, col, color_cliffs);
			tex2 = interpolate_color(grass, color_cliffs, col);
			col = interpolate_color(flatness, tex1, tex2);
		} else col = interpolate_color(flatness, color_cliffs, col);
		return col;
	}
	else if (n > 0.005) {
		n -= 0.005; n *= 200.0;
		color_cliffs = m_dirtColor[2];
		col = interpolate_color(equatorial_desert, m_darkplantColor[0], m_sandColor[1]);
		col = interpolate_color(n, col, m_plantColor[0]);
		if (textures) {
			tex1 = interpolate_color(rock, col, color_cliffs);
			tex2 = interpolate_color(grass, color_cliffs, col);
			col = interpolate_color(flatness, tex1, tex2);
		} else col = interpolate_color(flatness, color_cliffs, col);
		return col;
	}
	else {
		n *= 200.0;
		color_cliffs = m_darksandColor[0];
		col = interpolate_color(equatorial_desert, m_sandColor[0], m_sandColor[1]);
		col = interpolate_color(n, col, m_darkplantColor[0]);
		if (textures) {
			tex1 = interpolate_color(rock, col, color_cliffs);
			tex2 = interpolate_color(sand, col, color_cliffs);
			return col = interpolate_color(flatness, tex1, tex2);
		} else {
			return col = interpolate_color(flatness, color_cliffs, col);
		}
	}
}

