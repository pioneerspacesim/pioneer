#include "Terrain.h"
#include "TerrainNoise.h"
#include "TerrainComponent.h"

using namespace TerrainNoise;
using namespace TerrainComponent;

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
}

