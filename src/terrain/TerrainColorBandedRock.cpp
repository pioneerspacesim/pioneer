// Copyright © 2008-2014 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#include "Terrain.h"
#include "TerrainNoise.h"

using namespace TerrainNoise;

template <>
const char *TerrainColorFractal<TerrainColorBandedRock>::GetColorFractalName() const { return "BandedRock"; }

template <>
TerrainColorFractal<TerrainColorBandedRock>::TerrainColorFractal(const SystemBody *body) : Terrain(body)
{
}

template <>
void TerrainColorFractal<TerrainColorBandedRock>::GetColor(const vector3d &p, const double height, const vector3d &norm, Color3ub &out) const
{
	const double flatness = pow(p.Dot(norm), 6.0);
	double n = fabs(noise(vector3d(height*10000.0,0.0,0.0)));
	vector3d col = interpolate_color(n, m_rockColor[0], m_rockColor[1]);
	SetColour(out, interpolate_color(flatness, col, m_rockColor[2]));
}

