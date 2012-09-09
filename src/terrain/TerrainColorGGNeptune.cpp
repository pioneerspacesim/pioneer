#include "Terrain.h"
#include "TerrainNoise.h"
#include "TerrainFeature.h"

using namespace TerrainNoise;
using namespace TerrainFeature;

template <>
const char *TerrainColorFractal<TerrainColorGGNeptune>::GetColorFractalName() const { return "GGNeptune"; }

template <>
TerrainColorFractal<TerrainColorGGNeptune>::TerrainColorFractal(const SystemBody *body) : Terrain(body)
{
	double height = m_maxHeightInMeters*0.1;
	//spot boundary
	SetFracDef(0, height, 3e7, 10000000.0*m_fracmult);
	//spot
	SetFracDef(1, height, 9e7, 100.0*m_fracmult);
	//bands
	SetFracDef(2, height, 8e7, 1000.0*m_fracmult);
	SetFracDef(3, height, 1e8, 1000.0*m_fracmult);
}

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

