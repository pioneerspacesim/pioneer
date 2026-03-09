// Copyright © 2008-2026 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#include "Terrain.h"

#include "GameConfig.h"
#include "Pi.h"
#include "utils.h"
#include "../galaxy/SystemBody.h"

void Terrain::DebugDump() const
{
	Output("Terrain state dump:\n");
	Output("  Height fractal: %s\n", GetHeightFractalName());
	Output("  Color fractal: %s\n", GetColorFractalName());
	Output("  Config: DetailPlanets %d   FractalMultiple %d  Textures  %d\n", Pi::config->Int("DetailPlanets"), Pi::config->Int("FractalMultiple"), Pi::config->Int("Textures"));
	Output("  Seed: %d\n", m_seed);
	Output("  Body: %s [%d,%d,%d,%u,%u]\n", m_body->GetName().c_str(), m_body->GetPath().sectorX, m_body->GetPath().sectorY, m_body->GetPath().sectorZ, m_body->GetPath().systemIndex, m_body->GetPath().bodyIndex);
	Output("  Aspect Ratio: %g\n", m_body->GetAspectRatio());
	Output("  Fracdefs:\n");
	for (int i = 0; i < 10; i++) {
		Output("    %d: amp %f  freq %f  lac %f  oct %d\n", i, m_fracdef[i].amplitude, m_fracdef[i].frequency, m_fracdef[i].lacunarity, m_fracdef[i].octaves);
	}
}
