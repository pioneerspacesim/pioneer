// Copyright © 2008-2026 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#include "Terrain.h"
#include "TerrainFeature.h"
#include "TerrainNoise.h"

using namespace TerrainNoise;
using namespace TerrainFeature;

template <>
const char *TerrainHeightFractal<TerrainHeightHillsCraters>::GetHeightFractalName() const { return "HillsCraters"; }

template <>
TerrainHeightFractal<TerrainHeightHillsCraters>::TerrainHeightFractal(const SystemBody *body, const Uint32 surfaceEffects, const ETerrainColours terrainColour) :
	Terrain(body, surfaceEffects, terrainColour)
{
	SetFracDef(0, m_maxHeightInMeters, m_rand.Double(1e6, 1e7));
	double height = m_maxHeightInMeters * 0.3;
	SetFracDef(1, height, m_rand.Double(4.0, 20.0) * height);
	SetFracDef(2, m_maxHeightInMeters, m_rand.Double(50.0, 100.0) * m_maxHeightInMeters);
	SetFracDef(3, m_maxHeightInMeters * 0.07, 1e6, 100.0);
	SetFracDef(4, m_maxHeightInMeters * 0.05, 8e5, 100.0);
}

template <>
void TerrainHeightFractal<TerrainHeightHillsCraters>::GetHeights(const vector3d *vP, double *heightsOut, const size_t count) const
{
	for (size_t i = 0; i < count; i++) {
		const vector3d &p = vP[i];
		const double continents = octavenoise(m_fracdef[0], 0.5, p) - m_sealevel;
		if (continents < 0.0)
			heightsOut[i] = 0.0;

		// == TERRAIN_HILLS_NORMAL except river_octavenoise
		double n = 0.3 * continents;
		double distrib = river_octavenoise(m_fracdef[2], 0.5, p);
		double m = m_fracdef[1].amplitude * river_octavenoise(m_fracdef[1], 0.5 * distrib, p);

		// cliffs at shore
		if (continents < 0.001)
			n += m * continents * 1000.0f;
		else
			n += m;
		n += crater_function(m_fracdef[3], p);
		n += crater_function(m_fracdef[4], p);
		n *= m_maxHeight;
		heightsOut[i] = (n > 0.0 ? n : 0.0);
	}
}
