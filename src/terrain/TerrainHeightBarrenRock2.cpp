// Copyright © 2008-2026 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#include "Terrain.h"
#include "TerrainFeature.h"
#include "TerrainNoise.h"

using namespace TerrainNoise;
using namespace TerrainFeature;

// Strange world, looks like its been hit by thousands of years of erosion.
// Could be used as a basis for terrains that should have erosion.

template <>
const char *TerrainHeightFractal<TerrainHeightBarrenRock2>::GetHeightFractalName() const { return "Barren Rock 2"; }

template <>
TerrainHeightFractal<TerrainHeightBarrenRock2>::TerrainHeightFractal(const SystemBody *body) :
	Terrain(body)
{
}

template <>
void TerrainHeightFractal<TerrainHeightBarrenRock2>::GetHeights(const vector3d *vP, double *heightsOut, const size_t count) const
{
	for (size_t i = 0; i < count; i++) {
		const vector3d &p = vP[i];
		double n = billow_octavenoise(16, 0.3 * octavenoise(8, 0.4, 2.5, p), Clamp(5.0 * ridged_octavenoise(8, 0.377, 4.0, p), 1.0, 5.0), p);

		n *= m_maxHeight;
		heightsOut[i] = (n > 0.0 ? n : 0.0);
	}
}
