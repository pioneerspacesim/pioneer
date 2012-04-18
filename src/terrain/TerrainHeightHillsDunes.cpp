#include "Terrain.h"
#include "TerrainNoise.h"

using namespace TerrainNoise;

template <>
const char *TerrainHeightFractal<TerrainHeightHillsDunes>::GetHeightFractalName() const { return "HillsDunes"; }

template <>
TerrainHeightFractal<TerrainHeightHillsDunes>::TerrainHeightFractal(const SystemBody *body) : Terrain(body)
{
	//textures
	SetFracDef(0, m_maxHeightInMeters, m_rand.Double(50, 100), 10*m_fracmult);
	SetFracDef(1, m_maxHeightInMeters, m_rand.Double(300, 500), 10*m_fracmult);
	//small fractal/high detail
	SetFracDef(2, m_maxHeightInMeters*0.00000000001, 50, 50*m_fracmult);
	//continental:
	SetFracDef(3, m_maxHeightInMeters*0.00001, 1e7, 1000*m_fracmult); 
	//large fractal:
	SetFracDef(4, m_maxHeightInMeters*0.00001, 1e5, 200*m_fracmult); 
	SetFracDef(5, m_maxHeightInMeters*0.000001, 5e4, 100*m_fracmult); 
	SetFracDef(6, m_maxHeightInMeters*0.0000001, 1e4, 50*m_fracmult); 
	//medium fractal:
	SetFracDef(7, m_maxHeightInMeters*0.0000000002, 1e3, 20*m_fracmult); 
}

template <>
double TerrainHeightFractal<TerrainHeightHillsDunes>::GetHeight(const vector3d &p)
{
	double continents = ridged_octavenoise(GetFracDef(3), 0.65, p) * (1.0-m_sealevel) - (m_sealevel*0.1);
	if (continents < 0) return 0;
	double n = continents;
	double distrib = dunes_octavenoise(GetFracDef(4), 0.4, p);
	distrib *= distrib * distrib;
	double m = octavenoise(GetFracDef(7), 0.5, p) * dunes_octavenoise(GetFracDef(7), 0.5, p)
		* Clamp(0.2-distrib, 0.0, 0.05);
	m += octavenoise(GetFracDef(2), 0.5, p) * dunes_octavenoise(GetFracDef(2), 0.5
	*octavenoise(GetFracDef(6), 0.5*distrib, p), p) * Clamp(1.0-distrib, 0.0, 0.0005);
	double mountains = ridged_octavenoise(GetFracDef(5), 0.5*distrib, p) 
		* octavenoise(GetFracDef(4), 0.5*distrib, p) * octavenoise(GetFracDef(6), 0.5, p) * distrib;
	mountains *= mountains;
	m += mountains;
	//detail for mountains, stops them looking smooth.
	//m += mountains*mountains*0.02*octavenoise(GetFracDef(2), 0.6*mountains*mountains*distrib, p);
	//m *= m*m*m*10.0;
	// smooth cliffs at shore
	if (continents < 0.01) n += m * continents * 100.0f;
	else n += m;
	//n += continents*Clamp(0.5-m, 0.0, 0.5)*0.2*dunes_octavenoise(GetFracDef(6), 0.6*distrib, p);
	//n += continents*Clamp(0.05-n, 0.0, 0.01)*0.2*dunes_octavenoise(GetFracDef(2), Clamp(0.5-n, 0.0, 0.5), p);
	return (n > 0.0 ? n*m_maxHeight : 0.0); 
}
