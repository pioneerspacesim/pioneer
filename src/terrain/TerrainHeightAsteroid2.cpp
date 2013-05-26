// Copyright © 2008-2013 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#include "Terrain.h"
#include "TerrainNoise.h"
#include "TerrainFeature.h"

using namespace TerrainNoise;
using namespace TerrainFeature;

// Banded/Ridged pattern mountainous terrain, could pass for desert

template <>
const char *TerrainHeightFractal<TerrainHeightAsteroid2>::GetHeightFractalName() const { return "Asteroid"; }

template <>
TerrainHeightFractal<TerrainHeightAsteroid2>::TerrainHeightFractal(const SystemBody *body) : Terrain(body)
{
}

template <>
double TerrainHeightFractal<TerrainHeightAsteroid2>::GetHeight(const vector3d &p) const
{
	double n = (((ridged_octavenoise(6, 0.2*octavenoise(2, 0.3, 1.73, p), 2.5*octavenoise(2, 0.5, 1.9, p), p) *
		0.75 * voronoiscam_octavenoise(8*octavenoise(2, 0.275, 1.73, p), 0.4*ridged_octavenoise(4, 0.4, 2.13, p), 2.5*octavenoise(3, 0.35, 2.66, p), p))) * (m_maxHeight*0.2)) + m_maxHeight*0.5;

	return (n > 0.0? m_maxHeight*n : 0.0);
}
