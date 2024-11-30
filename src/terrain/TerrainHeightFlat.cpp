// Copyright © 2008-2024 Pioneer Developers. See AUTHORS.txt for details
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
double TerrainHeightFractal<TerrainHeightFlat>::GetHeight(const vector3d &p) const
{
	return 0.0;
}
