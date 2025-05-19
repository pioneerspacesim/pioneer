// Copyright © 2008-2025 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#include "Terrain.h"

template <>
const char *TerrainHeightFractal<TerrainHeightFlat>::GetHeightFractalName() const { return "Flat"; }

template <>
TerrainHeightFractal<TerrainHeightFlat>::TerrainHeightFractal(const SystemBody *body) :
	Terrain(body)
{
}

template <>
void TerrainHeightFractal<TerrainHeightFlat>::GetHeights(const std::vector<vector3d> &vP, std::vector<double> &heightsOut) const
{
	for (size_t i = 0; i < vP.size(); i++) {
		heightsOut.at(i) = 0.0;
	}
}
