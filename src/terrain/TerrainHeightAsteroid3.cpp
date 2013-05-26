// Copyright © 2008-2013 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#include "Terrain.h"
#include "TerrainNoise.h"
#include "TerrainFeature.h"

using namespace TerrainNoise;
using namespace TerrainFeature;

// Banded/Ridged pattern mountainous terrain, could pass for desert

template <>
const char *TerrainHeightFractal<TerrainHeightAsteroid3>::GetHeightFractalName() const { return "Asteroid3"; }

template <>
TerrainHeightFractal<TerrainHeightAsteroid3>::TerrainHeightFractal(const SystemBody *body) : Terrain(body)
{
	SetFracDef(0, m_maxHeightInMeters*0.05, 1e6, 10000.0*m_fracmult);
	SetFracDef(1, m_maxHeightInMeters*0.04, 9e5, 10000.0*m_fracmult);
}

template <>
double TerrainHeightFractal<TerrainHeightAsteroid3>::GetHeight(const vector3d &p) const
{
	//return std::max(0.0, m_maxHeight*(octavenoise(8, 0.5, 2.3, p) * ridged_octavenoise(8, 0.5, 1.3, p)));
	return std::max(0.0, m_maxHeight*(octavenoise(GetFracDef(0), 0.5, p) * ridged_octavenoise(GetFracDef(1), 0.5,p)));
}
