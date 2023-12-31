// Copyright © 2008-2024 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#include "Terrain.h"
#include "TerrainFeature.h"
#include "TerrainNoise.h"

using namespace TerrainNoise;
using namespace TerrainFeature;

// Cool terrain for asteroids or small planets

template <>
const char *TerrainHeightFractal<TerrainHeightAsteroid4>::GetHeightFractalName() const { return "Asteroid4"; }

template <>
TerrainHeightFractal<TerrainHeightAsteroid4>::TerrainHeightFractal(const SystemBody *body) :
	Terrain(body)
{
	SetFracDef(0, m_maxHeightInMeters * 0.05, 1e6, 10000.0);
	const double height = m_maxHeightInMeters * 0.3;
	SetFracDef(1, height, m_rand.Double(4.0, 20.0) * height);
	SetFracDef(2, m_maxHeightInMeters, m_rand.Double(50.0, 100.0) * m_maxHeightInMeters);
	SetFracDef(3, m_maxHeightInMeters * 0.07, 1e6, 100.0);
	SetFracDef(4, m_maxHeightInMeters * 0.05, 8e5, 100.0);
}

template <>
double TerrainHeightFractal<TerrainHeightAsteroid4>::GetHeight(const vector3d &p) const
{
	const double n = octavenoise(6, 0.2 * octavenoise(GetFracDef(0), 0.3, p), 2.8 * ridged_octavenoise(GetFracDef(1), 0.5, p), p) *
		0.75 * ridged_octavenoise(16 * octavenoise(GetFracDef(2), 0.275, p), 0.3 * octavenoise(GetFracDef(3), 0.4, p), 2.8 * ridged_octavenoise(GetFracDef(4), 0.35, p), p);

	return (n > 0.0 ? m_maxHeight * n : 0.0);
}
