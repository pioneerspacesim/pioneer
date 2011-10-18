#include "Terrain.h"
#include "TerrainNoise.h"
#include "TerrainComponent.h"

using namespace TerrainNoise;
using namespace TerrainComponent;

template <>
double TerrainHeightFractal<TerrainHeightMountainsCraters2>::GetHeight(const vector3d &p)
{
	double continents = octavenoise(GetFracDef(0), 0.5, p) - m_sealevel;
	if (continents < 0) return 0;
	double n = 0.3 * continents;
	double m = 0;//GetFracDef(1).amplitude * octavenoise(GetFracDef(1), 0.5, p);
	double distrib = 0.5*ridged_octavenoise(GetFracDef(1), 0.5*octavenoise(GetFracDef(2), 0.5, p), p);
	distrib += 0.7*billow_octavenoise(GetFracDef(2), 0.5*ridged_octavenoise(GetFracDef(1), 0.5, p), p) +
		0.1*octavenoise(GetFracDef(3), 0.5*ridged_octavenoise(GetFracDef(2), 0.5, p), p);

	if (distrib > 0.5) m += 2.0 * (distrib-0.5) * GetFracDef(3).amplitude * octavenoise(GetFracDef(4), 0.5*distrib, p);
	// cliffs at shore
	if (continents < 0.001) n += m * continents * 1000.0f;
	else n += m;
	n += crater_function(GetFracDef(5), p);
	n += crater_function(GetFracDef(6), p);
	n += crater_function(GetFracDef(7), p);
	n += crater_function(GetFracDef(8), p);
	n += crater_function(GetFracDef(9), p);
	n *= m_maxHeight;
	return (n > 0.0 ? n : 0.0);
}
