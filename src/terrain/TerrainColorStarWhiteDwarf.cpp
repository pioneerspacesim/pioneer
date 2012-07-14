#include "Terrain.h"
#include "TerrainNoise.h"

using namespace TerrainNoise;

template <>
const char *TerrainColorFractal<TerrainColorStarWhiteDwarf>::GetColorFractalName() const { return "StarWhiteDwarf"; }

template <>
TerrainColorFractal<TerrainColorStarWhiteDwarf>::TerrainColorFractal(const SystemBody *body) : Terrain(body)
{
	double height = m_maxHeightInMeters*0.1;
	SetFracDef(0, height, 3e9, 100.0*m_fracmult); //why on Earth we need a feature size of 3,000,000 KM (2.2x the sun) I don't know, but we do... :)
	SetFracDef(1, height, 1e7, 100.0*m_fracmult);
	//Original settings with correct feature size, does not seem to work anymore:
	//SetFracDef(0, height, 3e5, 10.0*m_fracmult);
	//SetFracDef(1, height, 1e5, 10.0*m_fracmult);
}

template <>
vector3d TerrainColorFractal<TerrainColorStarWhiteDwarf>::GetColor(const vector3d &p, double height, const vector3d &norm)
{
	double n;
	vector3d col;
	n = ridged_octavenoise(GetFracDef(0), 0.8, p*p.x);
	n += ridged_octavenoise(GetFracDef(1), 0.8, p);
	n += voronoiscam_octavenoise(GetFracDef(0), 0.8 * octavenoise(GetFracDef(1), 0.6, p), p);
	n *= n*n;
	if (n > 0.666) {
		n -= 0.666; n *= 3.0;
		col = interpolate_color(n, vector3d(.8, .8, 1.0), vector3d(1.0, 1.0, 1.0));
		return col;
	} else if (n > 0.333) {
		n -= 0.333; n *= 3.0;
		col = interpolate_color(n, vector3d(.6, .8, .8), vector3d(.8, .8, 1.0));
		return col;
	} else {
		n *= 3.0;
		col = interpolate_color(n, vector3d(.0, .0, .9), vector3d(.6, .8, .8));
		return col;
	}
}

