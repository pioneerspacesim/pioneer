#include "Terrain.h"
#include "TerrainNoise.h"

using namespace TerrainNoise;

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

