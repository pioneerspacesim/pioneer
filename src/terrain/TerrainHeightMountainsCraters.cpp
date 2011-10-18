#include "Terrain.h"
#include "TerrainNoise.h"
#include "TerrainComponent.h"

using namespace TerrainNoise;
using namespace TerrainComponent;

template <>
double TerrainHeightFractal<TerrainHeightMountainsCraters>::GetHeight(const vector3d &p)
{
	double continents = octavenoise(GetFracDef(0), 0.5, p) - m_sealevel;
	if (continents < 0) return 0;
	double n = 0.3 * continents;
	double m = GetFracDef(1).amplitude * ridged_octavenoise(GetFracDef(1), 0.5, p);
	double distrib = ridged_octavenoise(GetFracDef(4), 0.5, p);
	if (distrib > 0.5) m += 2.0 * (distrib-0.5) * GetFracDef(3).amplitude * ridged_octavenoise(GetFracDef(3), 0.5*distrib, p);
	// cliffs at shore
	if (continents < 0.001) n += m * continents * 1000.0f;
	else n += m;
	n += crater_function(GetFracDef(5), p);
	n += crater_function(GetFracDef(6), p);
	n *= m_maxHeight;
	return (n > 0.0 ? n : 0.0);
}

template <>
void TerrainHeightFractal<TerrainHeightMountainsCraters>::InitFracDef(MTRand &rand)
{
}
