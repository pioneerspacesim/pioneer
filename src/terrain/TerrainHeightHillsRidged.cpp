#include "Terrain.h"
#include "TerrainNoise.h"

using namespace TerrainNoise;

template <>
double TerrainHeightFractal<TerrainHeightHillsRidged>::GetHeight(const vector3d &p)
{
	double continents = ridged_octavenoise(GetFracDef(3), 0.65, p) * (1.0-m_sealevel) - (m_sealevel*0.1);
	if (continents < 0) return 0;
	double n = continents;
	double distrib = river_octavenoise(GetFracDef(4), 0.5, p);
	double m = 0.5* ridged_octavenoise(GetFracDef(4), 0.55*distrib, p);
	m += continents*0.25*ridged_octavenoise(GetFracDef(5), 0.58*distrib, p);
	// **
	m += 0.001*ridged_octavenoise(GetFracDef(6), 0.55*distrib*m, p);
	// cliffs at shore
	if (continents < 0.01) n += m * continents * 100.0f;
	else n += m;
	// was n -= 0.001*ridged_octavenoise(GetFracDef(6), 0.55*distrib*m, p);
	//n += 0.001*ridged_octavenoise(GetFracDef(6), 0.55*distrib*m, p);
	return (n > 0.0 ? n*m_maxHeight : 0.0);
}
