#include "Terrain.h"
#include "TerrainNoise.h"
#include "TerrainFeature.h"

using namespace TerrainNoise;
using namespace TerrainFeature;

template <>
const char *TerrainColorFractal<TerrainColorStarG>::GetColorFractalName() const { return "StarG"; }

template <>
TerrainColorFractal<TerrainColorStarG>::TerrainColorFractal(const SystemBody *body) : Terrain(body)
{
	double height = m_maxHeightInMeters*0.1;
	SetFracDef(0, height, 8e8, 1000.0*m_fracmult);
	SetFracDef(1, height, 7e8, 10000.0*m_fracmult);
	SetFracDef(2, height, 4e6, 100.0*m_fracmult);
	SetFracDef(3, height, 2e5, 100.0*m_fracmult);
}

template <>
vector3d TerrainColorFractal<TerrainColorStarG>::GetColor(const vector3d &p, double height, const vector3d &norm)
{
	double n;
	vector3d col;
	n = octavenoise(GetFracDef(0), 0.5, p) * 0.5;
	n += voronoiscam_octavenoise(GetFracDef(1), 0.5, p) * 0.5;
	n += octavenoise(GetFracDef(0), 0.5, p) * billow_octavenoise(GetFracDef(1), 0.5, p);
	n += octavenoise(GetFracDef(2), 0.5, p) * 0.5 * Clamp(GetFracDef(0).amplitude-0.2, 0.0, 1.0);
	n += 15.0*billow_octavenoise(GetFracDef(0), 0.8, noise(p*3.142)*p)*
	 megavolcano_function(GetFracDef(1), p);
	n *= n * 0.15;
	n = 1.0-n;
	if (n > 0.666) {
		//n -= 0.666; n *= 3.0;
		//col = interpolate_color(n, vector3d(1.0, 1.0, 1.0), vector3d(1.0, 1.0, 1.0) );
		col = vector3d(1.0, 1.0, 1.0);
		return col;
	} else if (n > 0.333) {
		n -= 0.333; n *= 3.0;
		col = interpolate_color(n, vector3d(.6, .6, .0), vector3d(1.0, 1.0, 1.0) );
		return col;
	} else if (n > 0.05) {
		n -= 0.05;
		n *= 3.533;	
		col = interpolate_color(n, vector3d(.8, .8, .0), vector3d(.6, .6, .0) );
		return col;
	} else {
		n *= 20.0;
		col = interpolate_color(n, vector3d(.02, .0, .0), vector3d(.8, .8, .0) );
		return col;
	}
}

