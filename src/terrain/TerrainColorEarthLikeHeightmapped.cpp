// Copyright © 2008-2020 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#include "Terrain.h"

template <>
const char *TerrainColorFractal<TerrainColorEarthLikeHeightmapped>::GetColorFractalName() const { return "EarthLikeHeightmapped"; }

template <>
TerrainColorFractal<TerrainColorEarthLikeHeightmapped>::TerrainColorFractal(const SystemBody *body) :
	Terrain(body)
{
	// crappy water
	//double height = m_maxHeightInMeters*0.5;
	//SetFracDef(3, m_maxHeightInMeters, 1e8, 50.0);
	//SetFracDef(2, m_maxHeightInMeters, 10, 10.0);
	m_surfaceEffects |= Terrain::EFFECT_WATER;
}
