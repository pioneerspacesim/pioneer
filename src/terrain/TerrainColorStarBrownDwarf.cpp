#include "Terrain.h"
#include "TerrainNoise.h"

using namespace TerrainNoise;

template <>
const char *TerrainColorFractal<TerrainColorStarBrownDwarf>::GetColorFractalName() const { return "StarBrownDwarf"; }

template <>
TerrainColorFractal<TerrainColorStarBrownDwarf>::TerrainColorFractal(const SBody *body) : Terrain(body)
{
	double height = m_maxHeightInMeters*0.1;
	SetFracDef(0, height, 5e6, 10.0*m_fracmult);
	SetFracDef(1, height, 3e6, 10.0*m_fracmult);
	SetFracDef(2, height, 1e5, 10.0*m_fracmult);
	SetFracDef(3, height, 1e2, 10.0*m_fracmult);
}

template <>
vector3d TerrainColorFractal<TerrainColorStarBrownDwarf>::GetColor(const vector3d &p, double height, const vector3d &norm)
{
	double n;
	vector3d col;
	n = voronoiscam_octavenoise(GetFracDef(0), 0.6, p) * 0.5;
	//n += ridged_octavenoise(GetFracDef(1), 0.7, p) * 0.5;
	//n += voronoiscam_octavenoise(GetFracDef(0), 0.8, p) * ridged_octavenoise(GetFracDef(1), 0.8, p);
	//n *= n * n;
	//n += voronoiscam_octavenoise(GetFracDef(2), 0.6, p) * 0.5;
	//n += ridged_octavenoise(GetFracDef(3), 0.6, p) * 0.5;
	//n += 0.8*billow_octavenoise(GetFracDef(0), 0.8, noise(p*3.142)*p);
	//n *= n;
	if (n > 0.666) {
		n -= 0.666; n *= 3.0;
		col = interpolate_color(n, vector3d(.25, .2, .2), vector3d(.1, .0, .0) );
		return col;
	} else if (n > 0.333) {
		n -= 0.333; n *= 3.0;
		col = interpolate_color(n, vector3d(.2, .25, .1), vector3d(.25, .2, .2) );
		return col;
	} else {
		n *= 3.0;
		col = interpolate_color(n, vector3d(1.5, 1.0, 1.0), vector3d(.2, .25, .1) );
		return col;
	}
}

