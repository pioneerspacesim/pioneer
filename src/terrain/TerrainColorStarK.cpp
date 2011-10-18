#include "Terrain.h"
#include "TerrainNoise.h"

using namespace TerrainNoise;

template <>
vector3d TerrainColorFractal<TerrainColorStarK>::GetColor(const vector3d &p, double height, const vector3d &norm)
{
	double n;
	vector3d col;
	n = octavenoise(GetFracDef(0), 0.6, p) * 0.5;
	n += ridged_octavenoise(GetFracDef(1), 0.7, p) * 0.5;
	n += billow_octavenoise(GetFracDef(0), 0.8, p) * octavenoise(GetFracDef(1), 0.8, p);
	n -= dunes_octavenoise(GetFracDef(2), 0.6, p) * 0.5;
	n += octavenoise(GetFracDef(3), 0.6, p) * 0.5;
	n *= n * 0.2;
	if (n > 0.666) {
		n -= 0.666; n *= 3.0;
		col = interpolate_color(n, vector3d(.95, .7, .25), vector3d(1.0, 1.0, 1.0) );
		return col;
	} else if (n > 0.333) {
		n -= 0.333; n *= 3.0;
		col = interpolate_color(n, vector3d(.4, .25, .0), vector3d(.95, .7, .25) );
		return col;
	} else {
		n *= 3.0;
		col = interpolate_color(n, vector3d(.2, .05, .0), vector3d(.4, .25, .0) );
		return col;
	}
}

