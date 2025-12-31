// Copyright © 2008-2026 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#include "Terrain.h"
#include "TerrainFeature.h"
#include "TerrainNoise.h"

using namespace TerrainNoise;
using namespace TerrainFeature;

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

template <>
vector3d TerrainColorFractal<TerrainColorGGSaturn>::GetColor(const vector3d &p, double height, const vector3d &norm) const
{
	double n = 0.4 * ridged_octavenoise(m_fracdef[0], 0.7, vector3d(3.142 * p.y * p.y));
	n += 0.4 * octavenoise(m_fracdef[1], 0.6, vector3d(3.142 * p.y * p.y));
	n += 0.3 * octavenoise(m_fracdef[2], 0.5, vector3d(3.142 * p.y * p.y));
	n += 0.8 * octavenoise(m_fracdef[0], 0.7, vector3d(p * p.y * p.y));
	n += 0.5 * ridged_octavenoise(m_fracdef[1], 0.7, vector3d(p * p.y * p.y));
	n /= 2.0;
	n *= n * n;
	n += billow_octavenoise(m_fracdef[0], 0.8, vector3d(noise(p * 3.142) * p)) *
		megavolcano_function(m_fracdef[3], p);
	return interpolate_color(n, vector3d(.69, .53, .43), vector3d(.99, .76, .62));
}
