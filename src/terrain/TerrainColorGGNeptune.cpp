// Copyright © 2008-2020 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#include "Terrain.h"

template <>
const char *TerrainColorFractal<TerrainColorGGNeptune>::GetColorFractalName() const { return "GGNeptune"; }

template <>
TerrainColorFractal<TerrainColorGGNeptune>::TerrainColorFractal(const SystemBody *body) :
	Terrain(body)
{
	const double height = m_maxHeightInMeters * 0.1;
	//spot boundary
	SetFracDef(0, height, 3e7, 10000000.0);
	//spot
	SetFracDef(1, height, 9e7, 100.0);
	//bands
	SetFracDef(2, height, 8e7, 1000.0);
	SetFracDef(3, height, 1e8, 1000.0);
}
