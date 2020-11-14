// Copyright © 2008-2020 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#include "Terrain.h"

template <>
const char *TerrainColorFractal<TerrainColorStarWhiteDwarf>::GetColorFractalName() const { return "StarWhiteDwarf"; }

template <>
TerrainColorFractal<TerrainColorStarWhiteDwarf>::TerrainColorFractal(const SystemBody *body) :
	Terrain(body)
{
	double height = m_maxHeightInMeters * 0.1;
	SetFracDef(0, height, 3e9, 100.0); //why on Earth we need a feature size of 3,000,000 KM (2.2x the sun) I don't know, but we do... :)
	SetFracDef(1, height, 1e7, 100.0);
	//Original settings with correct feature size, does not seem to work anymore:
	//SetFracDef(0, height, 3e5, 10.0*m_fracmult);
	//SetFracDef(1, height, 1e5, 10.0*m_fracmult);
}
