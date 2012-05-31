#include "Terrain.h"
#include "TerrainNoise.h"
#include "TerrainFeature.h"

using namespace TerrainNoise;
using namespace TerrainFeature;

template <>
const char *TerrainHeightFractal<TerrainHeightWaterSolidCanyons>::GetHeightFractalName() const { return "WaterSolidCanyons"; }

template <>
TerrainHeightFractal<TerrainHeightWaterSolidCanyons>::TerrainHeightFractal(const SystemBody *body) : Terrain(body)
{
	SetFracDef(0, m_maxHeightInMeters, m_rand.Double(5e6,1e8));
	double height = m_maxHeightInMeters*0.3;
	SetFracDef(1, height, m_rand.Double(4.0, 20.0)*height);
	SetFracDef(2, m_maxHeightInMeters, m_rand.Double(200.0, 1000.0)*m_maxHeightInMeters);

	// mountains with some canyons
	SetFracDef(3, m_maxHeightInMeters*0.4, 4e6);
	SetFracDef(4, m_maxHeightInMeters*0.4, 5e6);
	//crater
	SetFracDef(5, m_maxHeightInMeters*0.4, 15e6, 50000.0*m_fracmult);
	//canyons
	//SetFracDef(6, m_maxHeightInMeters*0.4, 12e6, 50000.0);
	//SetFracDef(7, m_maxHeightInMeters*0.4, 9e6, 50000.0);
}

template <>
double TerrainHeightFractal<TerrainHeightWaterSolidCanyons>::GetHeight(const vector3d &p)
{
	double continents = 0.7*river_octavenoise(GetFracDef(2), 0.5, p)-m_sealevel;
	continents = GetFracDef(0).amplitude * ridged_octavenoise(GetFracDef(0), 
		Clamp(continents, 0.0, 0.6), p);
	double mountains = ridged_octavenoise(GetFracDef(2), 0.5, p);
	double hills = octavenoise(GetFracDef(2), 0.5, p) *
		GetFracDef(1).amplitude * river_octavenoise(GetFracDef(1), 0.5, p);
	double n = continents - (GetFracDef(0).amplitude*m_sealevel);
	if (n > 0.0) {
		// smooth in hills at shore edges 
		if (n < 0.05) {
			n += hills * n * 4.0 ;
			n += n * 20.0 * (billow_octavenoise(GetFracDef(3), 0.5*
				ridged_octavenoise(GetFracDef(2), 0.5, p), p) +
				river_octavenoise(GetFracDef(4), 0.5*
				ridged_octavenoise(GetFracDef(3), 0.5, p), p) +
				billow_octavenoise(GetFracDef(3), 0.6*
				ridged_octavenoise(GetFracDef(4), 0.55, p), p));
		} else {
			n += hills * .2f ;
			n += billow_octavenoise(GetFracDef(3), 0.5*
				ridged_octavenoise(GetFracDef(2), 0.5, p), p) +
				river_octavenoise(GetFracDef(4), 0.5*
				ridged_octavenoise(GetFracDef(3), 0.5, p), p) +
				billow_octavenoise(GetFracDef(3), 0.6*
				ridged_octavenoise(GetFracDef(4), 0.55, p), p);
		}
		// adds mountains hills craters 
		mountains = octavenoise(GetFracDef(3), 0.5, p) *
			GetFracDef(2).amplitude * mountains*mountains*mountains;
		if (n < 0.4) n += 2.0 * n * mountains;
		else n += mountains * .8f;
	}	
	// craters
	n += 3.0*impact_crater_function(GetFracDef(5), p);	
	n = m_maxHeight*n;
	n = (n<0.0 ? 0 : n);
	n = (n>1.0 ? 2.0-n : n);
	return n;
}
