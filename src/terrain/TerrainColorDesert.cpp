// Copyright © 2008-2026 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#include "Terrain.h"
#include "TerrainNoise.h"

using namespace TerrainNoise;

template <>
const char *TerrainColorFractal<TerrainColorDesert>::GetColorFractalName() const { return "Desert"; }

template <>
TerrainColorFractal<TerrainColorDesert>::TerrainColorFractal(const SystemBody *body) :
	Terrain(body)
{
}
