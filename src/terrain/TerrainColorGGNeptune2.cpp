// Copyright © 2008-2020 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#include "Terrain.h"

template <>
const char *TerrainColorFractal<TerrainColorGGNeptune2>::GetColorFractalName() const { return "GGNeptune2"; }

template <>
TerrainColorFractal<TerrainColorGGNeptune2>::TerrainColorFractal(const SystemBody *body) :
	Terrain(body)
{
	// spots
	const double height = m_maxHeightInMeters * 0.1;
	SetFracDef(0, height, 2e8, 1000.0);
	SetFracDef(1, height, 9e7, 1000.0);
	SetFracDef(2, height, 6e7, 1000.0);
	SetFracDef(3, height, 1e8, 100.0);
}
