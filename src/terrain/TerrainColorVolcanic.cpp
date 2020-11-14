// Copyright © 2008-2020 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#include "Terrain.h"

template <>
const char *TerrainColorFractal<TerrainColorVolcanic>::GetColorFractalName() const { return "Volcanic"; }

template <>
TerrainColorFractal<TerrainColorVolcanic>::TerrainColorFractal(const SystemBody *body) :
	Terrain(body)
{
	// 50 percent chance of there being exposed lava
	if (m_rand.Int32(100) > 50)
		m_surfaceEffects |= Terrain::EFFECT_LAVA;
}
