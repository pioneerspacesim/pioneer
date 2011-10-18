#include "Terrain.h"
#include "TerrainNoise.h"
#include "TerrainComponent.h"

using namespace TerrainNoise;
using namespace TerrainComponent;

template <>
double TerrainHeightFractal<TerrainHeightRuggedDesert>::GetHeight(const vector3d &p)
{
	double continents = octavenoise(GetFracDef(0), 0.5, p) - m_sealevel;// + (cliff_function(GetFracDef(7), p)*0.5);
	if (continents < 0) return 0;
	double mountain_distrib = octavenoise(GetFracDef(2), 0.5, p);
	double mountains = ridged_octavenoise(GetFracDef(1), 0.5, p);
	//double rocks = octavenoise(GetFracDef(9), 0.5, p);
	double hill_distrib = octavenoise(GetFracDef(4), 0.5, p);
	double hills = hill_distrib * GetFracDef(3).amplitude * billow_octavenoise(GetFracDef(3), 0.5, p);
	double dunes = hill_distrib * GetFracDef(5).amplitude * dunes_octavenoise(GetFracDef(5), 0.5, p);
	double n = continents * GetFracDef(0).amplitude * 2 ;//+ (cliff_function(GetFracDef(6), p)*0.5);
	double m = canyon_normal_function(GetFracDef(6), p);
	m += canyon2_normal_function(GetFracDef(6), p);
	m += canyon3_ridged_function(GetFracDef(6), p);
	m = (n<1 ? n : 1/n ); //sometimes creates some interesting features
	m += canyon_billow_function(GetFracDef(7), p);
	m += canyon2_ridged_function(GetFracDef(7), p);
	m += canyon3_normal_function(GetFracDef(7), p);
	m += canyon_normal_function(GetFracDef(8), p);
	m += canyon2_ridged_function(GetFracDef(8), p);
	m += canyon3_voronoi_function(GetFracDef(8), p);
	m += -0.5;
	m = n * 0.5;
	m = (n<0.0 ? 0.0 : n);
	n += m;

	// makes larger dunes at lower altitudes, flat ones at high altitude.
	mountains = mountain_distrib * GetFracDef(3).amplitude * mountains*mountains*mountains;
	// smoothes edges of mountains and places them only above a set altitude
	if (n < 0.1) n += n * 10.0f * hills;
	else n += hills;
	if (n > 0.2) n += dunes * (0.2/n);  
	else n += dunes;
	if (n < 0.1) n += n * 10.0f * mountains;
	else n += mountains;	
	
	
	//rocks = mountain_distrib * GetFracDef(9).amplitude * rocks*rocks*rocks;
	//n += rocks ;
	
	
	//n = (n<0.0 ? 0.0 : m_maxHeight*n);
	n = m_maxHeight*n;
	return n;
}
