// Copyright © 2008-2020 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#include "Terrain.h"

template <>
const char *TerrainColorFractal<TerrainColorStarG>::GetColorFractalName() const { return "StarG"; }

template <>
TerrainColorFractal<TerrainColorStarG>::TerrainColorFractal(const SystemBody *body) :
	Terrain(body)
{
	double height = m_maxHeightInMeters * 0.1;
	SetFracDef(0, height, 8e8, 1000.0);
	SetFracDef(1, height, 7e8, 10000.0);
	SetFracDef(2, height, 4e6, 100.0);
	SetFracDef(3, height, 2e5, 100.0);
}
