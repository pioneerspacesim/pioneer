// Copyright © 2008-2020 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#include "Terrain.h"

template <>
const char *TerrainColorFractal<TerrainColorGGSaturn>::GetColorFractalName() const { return "GGSaturn"; }

template <>
TerrainColorFractal<TerrainColorGGSaturn>::TerrainColorFractal(const SystemBody *body) :
	Terrain(body)
{
	const double height = m_maxHeightInMeters * 0.1;
	//spot + clouds
	SetFracDef(0, height, 3e7, 10.0);
	SetFracDef(1, height, 9e7, 1000.0);
	SetFracDef(2, height, 8e7, 100.0);
	//spot boundary
	SetFracDef(3, height, 3e7, 10000000.0);
}
