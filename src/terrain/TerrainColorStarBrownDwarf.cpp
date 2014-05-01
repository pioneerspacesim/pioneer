// Copyright © 2008-2014 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#include "Terrain.h"
#include "TerrainNoise.h"

using namespace TerrainNoise;

template <>
const char *TerrainColorFractal<TerrainColorStarBrownDwarf>::GetColorFractalName() const { return "StarBrownDwarf"; }

template <>
TerrainColorFractal<TerrainColorStarBrownDwarf>::TerrainColorFractal(const SystemBody *body) : Terrain(body)
{
	double height = m_maxHeightInMeters*0.1;
	SetFracDef(0, height, 5e8, 100.0*m_fracmult);

}

template <>
void TerrainColorFractal<TerrainColorStarBrownDwarf>::GetColor(const vector3d &p, const double height, const vector3d &norm, Color3ub &out) const
{
	double n;
	vector3d col;
	n = voronoiscam_octavenoise(GetFracDef(0), 0.6, p) * 0.5;
	if (n > 0.666) {
		n -= 0.666; n *= 3.0;
		col = interpolate_color(n, vector3d(.25, .2, .2), vector3d(.1, .0, .0) );
		SetColour(out, col);
		return;
	} else if (n > 0.333) {
		n -= 0.333; n *= 3.0;
		col = interpolate_color(n, vector3d(.2, .25, .1), vector3d(.25, .2, .2) );
		SetColour(out, col);
		return;
	} else {
		n *= 3.0;
		col = interpolate_color(n, vector3d(1.5, 1.0, 1.0), vector3d(.2, .25, .1) );
		SetColour(out, col);
		return;
	}
}

