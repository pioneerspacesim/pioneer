// Copyright © 2008-2013 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#include "Terrain.h"

template <>
const char *TerrainColorFractal<TerrainColorSolid>::GetColorFractalName() const { return "Solid"; }

template <>
TerrainColorFractal<TerrainColorSolid>::TerrainColorFractal(const SystemBody *body, const uint32_t tET) : Terrain(body, tET)
{
}

template <>
vector3d TerrainColorFractal<TerrainColorSolid>::GetColor(const vector3d &p, double height, const vector3d &norm) const
{
	return vector3d(1.0);
}
