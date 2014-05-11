// Copyright © 2008-2014 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#include "Terrain.h"

template <>
const char *TerrainColorFractal<TerrainColorSolid>::GetColorFractalName() const { return "Solid"; }

template <>
TerrainColorFractal<TerrainColorSolid>::TerrainColorFractal(const SystemBody *body) : Terrain(body)
{
}

template <>
void TerrainColorFractal<TerrainColorSolid>::GetColor(const vector3d &p, const double height, const vector3d &norm, Color3ub &out) const
{
	out = Color3ub::WHITE;
}
