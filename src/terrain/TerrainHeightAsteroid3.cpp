// Copyright © 2008-2024 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#include "Terrain.h"
#include "TerrainFeature.h"
#include "TerrainNoise.h"

using namespace TerrainNoise;
using namespace TerrainFeature;

// Banded/Ridged pattern mountainous terrain, could pass for desert

template <>
const char *TerrainHeightFractal<TerrainHeightAsteroid3>::GetHeightFractalName() const { return "Asteroid3"; }

template <>
TerrainHeightFractal<TerrainHeightAsteroid3>::TerrainHeightFractal(const SystemBody *body) :
	Terrain(body)
{
	SetFracDef(0, m_maxHeightInMeters * 0.05, 1e6, 10000.0);
	const double height = m_maxHeightInMeters * 0.3;
	SetFracDef(1, height, m_rand.Double(4.0, 20.0) * height);
}

template <>
double TerrainHeightFractal<TerrainHeightAsteroid3>::GetHeight(const vector3d &p) const
{
	const double n = octavenoise(GetFracDef(0), 0.5, p) * ridged_octavenoise(GetFracDef(1), 0.5, p);

	return (n > 0.0 ? m_maxHeight * n : 0.0);
}
