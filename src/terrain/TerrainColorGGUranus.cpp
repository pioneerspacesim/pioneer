// Copyright © 2008-2020 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#include "Terrain.h"

template <>
const char *TerrainColorFractal<TerrainColorGGUranus>::GetColorFractalName() const { return "GGUranus"; }

template <>
TerrainColorFractal<TerrainColorGGUranus>::TerrainColorFractal(const SystemBody *body) :
	Terrain(body)
{
	const double height = m_maxHeightInMeters * 0.1;
	SetFracDef(0, height, 3e7, 1000.0);
	SetFracDef(1, height, 9e7, 1000.0);
	SetFracDef(2, height, 8e7, 1000.0);
}
