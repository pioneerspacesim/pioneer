#include "Terrain.h"
#include "TerrainNoise.h"

using namespace TerrainNoise;

template <>
TerrainColorFractal<TerrainColorStarG>::TerrainColorFractal(const SBody *body) : Terrain(body)
{
	double height = m_maxHeightInMeters*0.1;
	SetFracDef(0, height, 8e8, 10.0*m_fracmult);
	SetFracDef(1, height, 7e6, 10.0*m_fracmult);
	SetFracDef(2, height, 4e6, 10.0*m_fracmult);
	SetFracDef(3, height, 2e6, 10.0*m_fracmult);
}

template <>
vector3d TerrainColorFractal<TerrainColorStarG>::GetColor(const vector3d &p, double height, const vector3d &norm)
{
	double n;
	vector3d col;
	n = voronoiscam_octavenoise(GetFracDef(0), 0.5, p) * 0.5;
	n += voronoiscam_octavenoise(GetFracDef(1), 0.5, p) * 0.5;
	n += river_octavenoise(GetFracDef(0), 0.5, p) * voronoiscam_octavenoise(GetFracDef(1), 0.5, p);
	n += river_octavenoise(GetFracDef(2), 0.5, p) * 0.5;
	n += voronoiscam_octavenoise(GetFracDef(3), 0.5, p) * 0.5;
	n *= n * n * n * 0.1;
	if (n > 0.666) {
		n -= 0.666; n *= 3.0;
		col = interpolate_color(n, vector3d(.9, .9, .05), vector3d(1.0, 1.0, 1.0) );
		return col;
	} else if (n > 0.333) {
		n -= 0.333; n *= 3.0;
		col = interpolate_color(n, vector3d(.6, .6, .0), vector3d(.9, .9, .05) );
		return col;
	} else {
		n *= 3.0;
		col = interpolate_color(n, vector3d(.8, .8, .0), vector3d(.6, .6, .0) );
		return col;
	}
}

