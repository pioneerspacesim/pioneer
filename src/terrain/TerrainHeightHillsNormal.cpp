// Copyright © 2008-2026 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#include "Terrain.h"
#include "TerrainNoise.h"

using namespace TerrainNoise;

template <>
const char *TerrainHeightFractal<TerrainHeightHillsNormal>::GetHeightFractalName() const { return "HillsNormal"; }

template <>
TerrainHeightFractal<TerrainHeightHillsNormal>::TerrainHeightFractal(const SystemBody *body) :
	Terrain(body)
{
	//textures
	SetFracDef(0, m_maxHeightInMeters, m_rand.Double(5, 15), 10);
	SetFracDef(1, m_maxHeightInMeters, m_rand.Double(20, 40), 10);
	//small fractal/high detail
	SetFracDef(2, m_maxHeightInMeters * 0.000000005, 500, 20);
	//continental:
	SetFracDef(3, m_maxHeightInMeters * 0.00001, 1e7, 1000);
	//large fractal:
	SetFracDef(4, m_maxHeightInMeters, 1e5, 200);
	//medium fractal:
	SetFracDef(5, m_maxHeightInMeters * 0.00005, 2e4, 200);
	SetFracDef(6, m_maxHeightInMeters * 0.000000005, 1000, 20);
}

template <>
void TerrainHeightFractal<TerrainHeightHillsNormal>::GetHeights(const vector3d *vP, double *heightsOut, const size_t count) const
{
	for (size_t i = 0; i < count; i++) {
		const vector3d &p = vP[i];
		double continents = octavenoise(m_fracdef[3], 0.65, p) * (1.0 - m_sealevel) - (m_sealevel * 0.1);
		if (continents < 0.0) {
			heightsOut[i] = 0.0;
			continue;
		}
		double n = continents;

		double distrib = octavenoise(m_fracdef[4], 0.5, p);
		distrib *= distrib;
		double m = 0.5 * m_fracdef[3].amplitude * octavenoise(m_fracdef[4], 0.55 * distrib, p) * m_fracdef[5].amplitude;
		m += 0.25 * billow_octavenoise(m_fracdef[5], 0.55 * distrib, p);
		//hill footings
		m -= octavenoise(m_fracdef[2], 0.6 * (1.0 - distrib), p) * Clamp(0.05 - m, 0.0, 0.05) * Clamp(0.05 - m, 0.0, 0.05);
		//hill footings
		m += voronoiscam_octavenoise(m_fracdef[6], 0.765 * distrib, p) * Clamp(0.025 - m, 0.0, 0.025) * Clamp(0.025 - m, 0.0, 0.025);
		// cliffs at shore
		if (continents < 0.01)
			n += m * continents * 100.0f;
		else
			n += m;

		heightsOut[i] = (n > 0.0 ? n * m_maxHeight : 0.0);
	}
}
