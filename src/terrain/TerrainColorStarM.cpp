#include "Terrain.h"
#include "TerrainNoise.h"
#include "TerrainFeature.h"

using namespace TerrainNoise;

template <>
const char *TerrainColorFractal<TerrainColorStarM>::GetColorFractalName() const { return "StarM"; }

template <>
TerrainColorFractal<TerrainColorStarM>::TerrainColorFractal(const SystemBody *body) : Terrain(body)
{
	double height = m_maxHeightInMeters*0.1;
	SetFracDef(0, height, 22e7, 1000.0*m_fracmult);
	SetFracDef(1, height, 2e8, 10000.0*m_fracmult);
	SetFracDef(2, height, 6e6, 100.0*m_fracmult);
	SetFracDef(3, height, 5e5, 100.0*m_fracmult);
}
using namespace TerrainFeature;

template <>
vector3d TerrainColorFractal<TerrainColorStarM>::GetColor(const vector3d &p, double height, const vector3d &norm)
{
	double n;
	vector3d col;
	n = ridged_octavenoise(GetFracDef(0), 0.6, p) * 0.5;
	n += ridged_octavenoise(GetFracDef(1), 0.7, p) * 0.5;
	n += ridged_octavenoise(GetFracDef(0), 0.8, p) * ridged_octavenoise(GetFracDef(1), 0.8, p);
	n *= n * n;
	n += ridged_octavenoise(GetFracDef(2), 0.6, p) * 0.5;
	n += ridged_octavenoise(GetFracDef(3), 0.6, p) * 0.5;
	n += 15.0*billow_octavenoise(GetFracDef(0), 0.8, noise(p*3.142)*p)*
	 megavolcano_function(GetFracDef(1), p);
	n *= 0.15;
	n = 1.0-n;
	if (n > 0.666) {
		n -= 0.666; n *= 3.0;
		col = interpolate_color(n, vector3d(.65, .5, .25), vector3d(1.0, 1.0, 1.0) );
		return col;
	} else if (n > 0.333) {
		n -= 0.333; n *= 3.0;
		col = interpolate_color(n, vector3d(.3, .1, .0), vector3d(.65, .5, .25) );
		return col;
	} else {
		n *= 3.0;
		col = interpolate_color(n, vector3d(.03, .0, .0), vector3d(.3, .1, .0) );
		return col;
	}
}

