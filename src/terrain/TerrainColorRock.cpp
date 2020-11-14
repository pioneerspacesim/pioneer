// Copyright © 2008-2020 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#include "Terrain.h"

template <>
const char *TerrainColorFractal<TerrainColorRock>::GetColorFractalName() const { return "Rock"; }

template <>
TerrainColorFractal<TerrainColorRock>::TerrainColorFractal(const SystemBody *body) :
	Terrain(body)
{
}
