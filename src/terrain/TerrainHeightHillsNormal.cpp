#include "Terrain.h"
#include "TerrainNoise.h"

using namespace TerrainNoise;

template <>
const char *TerrainHeightFractal<TerrainHeightHillsNormal>::GetHeightFractalName() const { return "HillsNormal"; }

template <>
TerrainHeightFractal<TerrainHeightHillsNormal>::TerrainHeightFractal(const SBody *body) : Terrain(body)
{
	//textures
	if (textures) {
		SetFracDef(0, m_maxHeightInMeters, m_rand.Double(5, 15), 10*m_fracmult);
		SetFracDef(1, m_maxHeightInMeters, m_rand.Double(20, 40), 10*m_fracmult);
	}
	//small fractal/high detail
	SetFracDef(2-m_fracnum, m_maxHeightInMeters*0.000000005, 500, 20*m_fracmult);
	//continental:
	SetFracDef(3-m_fracnum, m_maxHeightInMeters*0.00001, 1e7, 1000*m_fracmult);
	//large fractal:
	SetFracDef(4-m_fracnum, m_maxHeightInMeters, 1e5, 200*m_fracmult);
	//medium fractal:
	SetFracDef(5-m_fracnum, m_maxHeightInMeters*0.00005, 2e4, 200*m_fracmult);
	SetFracDef(6-m_fracnum, m_maxHeightInMeters*0.000000005, 1000, 20*m_fracmult);
}

template <>
double TerrainHeightFractal<TerrainHeightHillsNormal>::GetHeight(const vector3d &p)
{
	double continents = octavenoise(GetFracDef(3-m_fracnum), 0.65, p) * (1.0-m_sealevel) - (m_sealevel*0.1);
	if (continents < 0) return 0;
	double n = continents;
	double distrib = octavenoise(GetFracDef(4-m_fracnum), 0.5, p);
	distrib *= distrib;
	double m = 0.5*GetFracDef(3-m_fracnum).amplitude * octavenoise(GetFracDef(4-m_fracnum), 0.55*distrib, p) 
	           * GetFracDef(5-m_fracnum).amplitude;
	m += 0.25*billow_octavenoise(GetFracDef(5-m_fracnum), 0.55*distrib, p);
	//hill footings
	m -= octavenoise(GetFracDef(2-m_fracnum), 0.6*(1.0-distrib), p) 
         * Clamp(0.05-m, 0.0, 0.05) * Clamp(0.05-m, 0.0, 0.05);
	//hill footings
	m += voronoiscam_octavenoise(GetFracDef(6-m_fracnum), 0.765*distrib, p) 
         * Clamp(0.025-m, 0.0, 0.025) * Clamp(0.025-m, 0.0, 0.025);
	// cliffs at shore
	if (continents < 0.01) n += m * continents * 100.0f;
	else n += m;
	return (n > 0.0 ? n*m_maxHeight : 0.0); 
	return 0.0;
}
