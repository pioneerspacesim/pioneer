// Copyright © 2008-2020 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#include "Terrain.h"

template <>
const char *TerrainColorFractal<TerrainColorStarK>::GetColorFractalName() const { return "StarK"; }

template <>
TerrainColorFractal<TerrainColorStarK>::TerrainColorFractal(const SystemBody *body) :
	Terrain(body)
{
	double height = m_maxHeightInMeters * 0.1;
	SetFracDef(0, height, 2e9, 100.0);
	SetFracDef(1, height, 7e7, 100.0);
	SetFracDef(2, height, 1e6, 100.0);
	SetFracDef(3, height, 1e3, 100.0);
}
