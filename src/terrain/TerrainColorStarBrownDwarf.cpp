// Copyright © 2008-2020 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#include "Terrain.h"

template <>
const char *TerrainColorFractal<TerrainColorStarBrownDwarf>::GetColorFractalName() const { return "StarBrownDwarf"; }

template <>
TerrainColorFractal<TerrainColorStarBrownDwarf>::TerrainColorFractal(const SystemBody *body) :
	Terrain(body)
{
	double height = m_maxHeightInMeters * 0.1;
	SetFracDef(0, height, 5e8, 100.0);
}
