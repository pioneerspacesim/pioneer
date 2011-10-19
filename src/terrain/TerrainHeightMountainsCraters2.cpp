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

template <>
void TerrainHeightFractal<TerrainHeightMountainsCraters2>::InitFracDef(MTRand &rand)
{
	SetFracDef(0, m_maxHeightInMeters, rand.Double(1e6,1e7), rand);
	double height = m_maxHeightInMeters*0.5;
	SetFracDef(1, height, rand.Double(50.0, 200.0)*height, rand, 10*m_fracmult);
	SetFracDef(2, m_maxHeightInMeters, rand.Double(500.0, 5000.0)*m_maxHeightInMeters, rand);

	height = m_maxHeightInMeters*0.4;
	SetFracDef(3, height, rand.Double(2.5,3.5)*height, rand);
	SetFracDef(4, m_maxHeightInMeters, rand.Double(100.0, 200.0)*m_maxHeightInMeters, rand);
	SetFracDef(5, m_maxHeightInMeters*0.05, 1e6, rand, 10000.0*m_fracmult);
	SetFracDef(6, m_maxHeightInMeters*0.04, 9e5, rand, 10000.0*m_fracmult);
	SetFracDef(7, m_maxHeightInMeters*0.05, 8e5, rand, 10000.0*m_fracmult);
	SetFracDef(8, m_maxHeightInMeters*0.04, 11e5, rand, 10000.0*m_fracmult);
	SetFracDef(9, m_maxHeightInMeters*0.07, 12e5, rand, 10000.0*m_fracmult);
}
