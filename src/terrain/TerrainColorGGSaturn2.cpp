#include "Terrain.h"
#include "TerrainNoise.h"
#include "TerrainFeature.h"

using namespace TerrainNoise;
using namespace TerrainFeature;

template <>
const char *TerrainColorFractal<TerrainColorGGSaturn2>::GetColorFractalName() const { return "GGSaturn2"; }

template <>
TerrainColorFractal<TerrainColorGGSaturn2>::TerrainColorFractal(const SystemBody *body) : Terrain(body)
{
	double height = m_maxHeightInMeters*0.1;
	//spot + clouds
	SetFracDef(0, height, 3e7, 10.0*m_fracmult);
	SetFracDef(1, height, 9e7, 1000.0*m_fracmult);
	SetFracDef(2, height, 8e7, 100.0*m_fracmult);
	//spot boundary
	SetFracDef(3, height, 3e7, 10000000.0*m_fracmult);
}

template <>
vector3d TerrainColorFractal<TerrainColorGGSaturn2>::GetColor(const vector3d &p, double height, const vector3d &norm)
{
	double n = 0.2*billow_octavenoise(GetFracDef(0), 0.8, p*p.y*p.y);
	n += 0.5*ridged_octavenoise(GetFracDef(1), 0.7, p*p.y*p.y);
	n += 0.25*octavenoise(GetFracDef(2), 0.7, p*p.y*p.y);
	//spot
	n *= n*n*0.5;
	n += billow_octavenoise(GetFracDef(0), 0.8, noise(p*3.142)*p)*
		 megavolcano_function(GetFracDef(3), p);
	vector3d col;
	//col = interpolate_color(octavenoise(GetFracDef(2), 0.7, noise(p*3.142)*p), vector3d(.05, .0, .0), vector3d(.4,.0,.35));
	if (n > 1.0) {
		n -= 1.0;// n *= 5.0;
		col = interpolate_color(n, vector3d(.25, .3, .4), vector3d(.0, .2, .0) );
	} else if (n >0.8) {
		n -= 0.8; n *= 5.0;
		col = interpolate_color(n, vector3d(.0, .0, .15), vector3d(.25, .3, .4) );
		return col;
	} else if (n>0.6) {
		n -= 0.6; n*= 5.0;
		col = interpolate_color(n, vector3d(.0, .0, .1), vector3d(.0, .0, .15) );
		return col;
	} else if (n>0.4) {
		n -= 0.4; n*= 5.0;
		col = interpolate_color(n, vector3d(.05, .0, .05), vector3d(.0, .0, .1) );
		return col;
	} else if (n>0.2) {
		n -= 0.2; n*= 5.0;
		col = interpolate_color(n, vector3d(.0, .0, .1), vector3d(.05, .0, .05) );
		return col;
	} else {
		n *= 5.0;
		col = interpolate_color(n, vector3d(.0, .0, .0), vector3d(.0, .0, .1) );
		return col;
	}
	// never happens, just silencing a warning
	return col;
}

