// Copyright © 2008-2014 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#include "Terrain.h"
#include "TerrainNoise.h"

using namespace TerrainNoise;

template <>
const char *TerrainColorFractal<TerrainColorMethane>::GetColorFractalName() const { return "Methane"; }

template <>
TerrainColorFractal<TerrainColorMethane>::TerrainColorFractal(const SystemBody *body) : Terrain(body)
{
}

template <>
void TerrainColorFractal<TerrainColorMethane>::GetColor(const vector3d &p, const double height, const vector3d &norm, Color3ub &out) const
{
	double n = m_invMaxHeight*height;
	if (n <= 0) 
		SetColour(out, vector3d(.3,.0,.0));
	else 
		SetColour(out, interpolate_color(n, vector3d(.3,.2,.0), vector3d(.6,.3,.0)));
}

