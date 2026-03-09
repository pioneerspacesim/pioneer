// Copyright © 2008-2026 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#include "Terrain.h"

template <>
const char *TerrainHeightFractal<TerrainHeightFlat>::GetHeightFractalName() const { return "Flat"; }

template <>
TerrainHeightFractal<TerrainHeightFlat>::TerrainHeightFractal(const SystemBody *body, const Uint32 surfaceEffects, const ETerrainColours terrainColour) :
	Terrain(body, surfaceEffects, terrainColour)
{
}

template <>
void TerrainHeightFractal<TerrainHeightFlat>::GetHeights(const vector3d *vP, double *heightsOut, const size_t count) const
{
	for (size_t i = 0; i < count; i++) {
		heightsOut[i] = 0.0;
	}
}
