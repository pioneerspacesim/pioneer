// Copyright © 2008-2024 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#include "Terrain.h"
#include "TerrainNoise.h"

// As far as I can tell this is used for the Moon and ONLY the Moon - AndyC

using namespace TerrainNoise;
template <>
const char *TerrainHeightFractal<TerrainHeightMapped2>::GetHeightFractalName() const { return "Mapped2"; }

template <>
TerrainHeightFractal<TerrainHeightMapped2>::TerrainHeightFractal(const SystemBody *body) :
	Terrain(body)
{
}

template <>
double TerrainHeightFractal<TerrainHeightMapped2>::GetHeight(const vector3d &p) const
{
	double v = BiCubicInterpolation(p);

	v = v * m_heightScaling + m_minh; // v = v*height scaling+min height
	v *= m_invPlanetRadius;

	v += 0.1;
	double h = 1.5 * v * v * v * ridged_octavenoise(16, 4.0 * v, 4.0, p);
	h += 30000.0 * v * v * v * v * v * v * v * ridged_octavenoise(16, 5.0 * v, 20.0 * v, p);
	h += v;
	h -= 0.09;

	return (h > 0.0 ? h : 0.0);
}
