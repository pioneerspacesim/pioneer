// Copyright © 2008-2026 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#include "Terrain.h"
#include "TerrainNoise.h"

using namespace TerrainNoise;

template <>
const char *TerrainHeightFractal<TerrainHeightHillsRivers>::GetHeightFractalName() const { return "HillsRivers"; }

template <>
TerrainHeightFractal<TerrainHeightHillsRivers>::TerrainHeightFractal(const SystemBody *body) :
	Terrain(body)
{
	//textures
	SetFracDef(0, m_maxHeightInMeters, m_rand.Double(5, 15), 10);
	SetFracDef(1, m_maxHeightInMeters, m_rand.Double(20, 40), 10);
	//small fractal/high detail
	SetFracDef(2, m_maxHeightInMeters * 0.000000008, m_rand.Double(5, 70), 10);
	//continental:
	SetFracDef(3, m_maxHeightInMeters, m_rand.Double(1e6, 2e7), 10000);
	//large fractal:
	SetFracDef(4, m_maxHeightInMeters * 0.00001, 1e5, 1000);
	SetFracDef(5, m_maxHeightInMeters * 0.000001, m_rand.Double(1e5, 1e6), 100);
	//medium fractal:
	SetFracDef(6, m_maxHeightInMeters * 0.0000002, m_rand.Double(500, 2e4), 50);
}

template <>
void TerrainHeightFractal<TerrainHeightHillsRivers>::GetHeights(const vector3d *vP, double *heightsOut, const size_t count) const
{
	for (size_t i = 0; i < count; i++) {
		const vector3d &p = vP[i];
		double continents = river_octavenoise(m_fracdef[3], 0.65, p) * (1.0 - m_sealevel) - (m_sealevel * 0.1);
		if (continents < 0.0)
			heightsOut[i] = 0.0;
		double n = continents;

		double distrib = voronoiscam_octavenoise(m_fracdef[4], 0.5 * m_fracdef[5].amplitude, p);
		double m = 0.1 * m_fracdef[4].amplitude * river_octavenoise(m_fracdef[5], 0.5 * distrib, p);
		double mountains = ridged_octavenoise(m_fracdef[5], 0.5 * distrib, p) * billow_octavenoise(m_fracdef[5], 0.5, p) *
			voronoiscam_octavenoise(m_fracdef[4], 0.5 * distrib, p) * distrib;
		m += mountains;
		//detail for mountains, stops them looking smooth.
		m += mountains * mountains * 0.02 * ridged_octavenoise(m_fracdef[2], 0.6 * mountains * mountains * distrib, p);
		m *= m * m * m * 10.0;
		// smooth cliffs at shore
		if (continents < 0.01)
			n += m * continents * 100.0f;
		else
			n += m;
		n += continents * Clamp(0.5 - m, 0.0, 0.5) * 0.2 * river_octavenoise(m_fracdef[6], 0.6 * distrib, p);
		n += continents * Clamp(0.05 - n, 0.0, 0.01) * 0.2 * dunes_octavenoise(m_fracdef[2], Clamp(0.5 - n, 0.0, 0.5), p);
		n *= m_maxHeight;
		heightsOut[i] = (n > 0.0 ? n : 0.0);
	}
}
