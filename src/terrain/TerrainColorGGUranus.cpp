#include "Terrain.h"
#include "TerrainNoise.h"

using namespace TerrainNoise;

template <>
vector3d TerrainColorFractal<TerrainColorGGUranus>::GetColor(const vector3d &p, double height, const vector3d &norm)
{
	double n = 0.5*ridged_octavenoise(GetFracDef(0), 0.7, 3.142*p.y*p.y);
	n += 0.5*octavenoise(GetFracDef(1), 0.6, 3.142*p.y*p.y);
	n += 0.2*octavenoise(GetFracDef(2), 0.5, 3.142*p.y*p.y);
	n /= 2.0;
	n *= n*n;
	return interpolate_color(n, vector3d(.4, .5, .55), vector3d(.85,.95,.96));
}

