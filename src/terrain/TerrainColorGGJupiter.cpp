// Copyright © 2008-2020 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#include "Terrain.h"

template <>
const char *TerrainColorFractal<TerrainColorGGJupiter>::GetColorFractalName() const { return "GGJupiter"; }

template <>
TerrainColorFractal<TerrainColorGGJupiter>::TerrainColorFractal(const SystemBody *body) :
	Terrain(body)
{
	// spots
	const double height = m_maxHeightInMeters * 0.1;
	SetFracDef(0, height, 1e8, 1000.0);
	SetFracDef(1, height, 8e7, 1000.0);
	SetFracDef(2, height, 4e7, 1000.0);
	SetFracDef(3, height, 1e7, 100.0);
}
