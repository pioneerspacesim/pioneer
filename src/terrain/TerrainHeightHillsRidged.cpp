#include "Terrain.h"
#include "TerrainNoise.h"

using namespace TerrainNoise;

template <>
const char *TerrainHeightFractal<TerrainHeightHillsRidged>::GetHeightFractalName() const { return "HillsRidged"; }

template <>
TerrainHeightFractal<TerrainHeightHillsRidged>::TerrainHeightFractal(const SystemBody *body) : Terrain(body)
{
	//textures:
	SetFracDef(0, m_maxHeightInMeters, m_rand.Double(5, 15), 10*m_fracmult);
	SetFracDef(1, m_maxHeightInMeters, m_rand.Double(20, 40), 10*m_fracmult);
	//small fractal/high detail:
	SetFracDef(2, m_maxHeightInMeters*0.000000005, m_rand.Double(40, 80), 10*m_fracmult);
	//continental:
	SetFracDef(3, m_maxHeightInMeters*0.00001, m_rand.Double(1e6, 2e7), 1000*m_fracmult);
	//large fractal:
	SetFracDef(4, m_maxHeightInMeters, m_rand.Double(1e5, 5e6), 200*m_fracmult);
	//medium fractal:
	SetFracDef(5, m_maxHeightInMeters*0.00005, m_rand.Double(1e3, 5e4), 100*m_fracmult);
	SetFracDef(6, m_maxHeightInMeters*0.00000002, m_rand.Double(250, 1e3), 50*m_fracmult);
}

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
