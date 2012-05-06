#include "Terrain.h"
#include "TerrainNoise.h"
#include "TerrainFeature.h"

using namespace TerrainNoise;
using namespace TerrainFeature;

template <>
const char *TerrainHeightFractal<TerrainHeightRuggedDesert>::GetHeightFractalName() const { return "RuggedDesert"; }

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

template <>
TerrainHeightFractal<TerrainHeightRuggedDesert>::TerrainHeightFractal(const SystemBody *body) : Terrain(body)
{
	SetFracDef(0, 0.1*m_maxHeightInMeters, 2e6, 180e3*m_fracmult);
	double height = m_maxHeightInMeters*0.9;
	SetFracDef(1, height, m_rand.Double(120.0, 10000.0)*height, 100*m_fracmult);
	SetFracDef(2, m_maxHeightInMeters, m_rand.Double(1.0, 2.0)*m_maxHeightInMeters);

	height = m_maxHeightInMeters*0.3;
	SetFracDef(3, height, m_rand.Double(20.0,240.0)*height);
	SetFracDef(4, m_maxHeightInMeters, m_rand.Double(1.0, 2.0)*m_maxHeightInMeters);
	// dunes
	height = m_maxHeightInMeters*0.2;
	SetFracDef(5, height*0.1, m_rand.Double(5,75)*height, 10000.0*m_fracmult);
	// canyon
	SetFracDef(6, m_maxHeightInMeters*0.2, 1e6, 200.0*m_fracmult);
	SetFracDef(7, m_maxHeightInMeters*0.35, 1.5e6, 100.0*m_fracmult);
	SetFracDef(8, m_maxHeightInMeters*0.2, 3e6, 100.0*m_fracmult);

	//SetFracDef(9, m_maxHeightInMeters*0.1, 100, 10.0);
	// adds bumps to the landscape
	SetFracDef(9, height*0.0025, m_rand.Double(1,100), 100.0*m_fracmult);
}
