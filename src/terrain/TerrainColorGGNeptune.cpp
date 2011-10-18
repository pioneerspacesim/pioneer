#include "Terrain.h"
#include "TerrainNoise.h"
#include "TerrainComponent.h"

using namespace TerrainNoise;
using namespace TerrainComponent;

template <>
vector3d TerrainColorFractal<TerrainColorGGNeptune>::GetColor(const vector3d &p, double height, const vector3d &norm)
{
	double n = 0.8*octavenoise(GetFracDef(2), 0.6, 3.142*p.y*p.y);
	n += 0.25*ridged_octavenoise(GetFracDef(3), 0.55, 3.142*p.y*p.y);
	n += 0.2*octavenoise(GetFracDef(3), 0.5, 3.142*p.y*p.y);
	//spot
	n += 0.8*billow_octavenoise(GetFracDef(1), 0.8, noise(p*3.142)*p)*
		 megavolcano_function(GetFracDef(0), p);
	n /= 2.0;
	n *= n*n;
	return interpolate_color(n, vector3d(.04, .05, .15), vector3d(.80,.94,.96));
}

