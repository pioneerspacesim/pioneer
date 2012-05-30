#include "Terrain.h"
#include "TerrainNoise.h"
#include "TerrainFeature.h"

using namespace TerrainNoise;
using namespace TerrainFeature;

template <>
const char *TerrainHeightFractal<TerrainHeightMountainsRiversVolcano>::GetHeightFractalName() const { return "MountainsRiversVolcano"; }

template <>
TerrainHeightFractal<TerrainHeightMountainsRiversVolcano>::TerrainHeightFractal(const SystemBody *body) : Terrain(body)
{
	SetFracDef(0, m_maxHeightInMeters, m_rand.Double(1e6,1e7));
	double height = m_maxHeightInMeters*0.6;
	SetFracDef(1, m_maxHeightInMeters, m_rand.Double(50.0, 100.0)*m_maxHeightInMeters);
	SetFracDef(2, height, m_rand.Double(4.0, 20.0)*height);
	SetFracDef(3, m_maxHeightInMeters, m_rand.Double(120.0, 2000.0)*m_maxHeightInMeters, 20*m_fracmult);

	height = m_maxHeightInMeters*0.3;
	SetFracDef(4, m_maxHeightInMeters, m_rand.Double(100.0, 200.0)*m_maxHeightInMeters);
	SetFracDef(5, height, m_rand.Double(2.5,3.5)*height);
	SetFracDef(6, height, m_rand.Double(2.5,3.5)*height);
	// volcano
	SetFracDef(7, 20000.0, 5000000.0, 100.0*m_fracmult);

	// canyons and rivers
	SetFracDef(8, m_maxHeightInMeters*1.0, 4e6, 100.0*m_fracmult);
	SetFracDef(9, m_maxHeightInMeters*1.0, 5e6, 100.0*m_fracmult);
	//SetFracDef(10, m_maxHeightInMeters*0.5, 2e6, 100.0);
}

template <>
double TerrainHeightFractal<TerrainHeightMountainsRiversVolcano>::GetHeight(const vector3d &p)
{
	double continents = octavenoise(GetFracDef(0), 0.5, p) - m_sealevel;
	if (continents < 0) return 0;
	// unused variable \\ double mountain_distrib = octavenoise(GetFracDef(1), 0.5, p);
	double mountains = octavenoise(GetFracDef(2), 0.5, p);
	double mountains2 = octavenoise(GetFracDef(3), 0.5, p);
	double hill_distrib = octavenoise(GetFracDef(4), 0.5, p);
	double hills = hill_distrib * GetFracDef(5).amplitude * octavenoise(GetFracDef(5), 0.5, p);
	double hills2 = hill_distrib * GetFracDef(6).amplitude * octavenoise(GetFracDef(6), 0.5, p);



	double n = continents - (GetFracDef(0).amplitude*m_sealevel);

	
	if (n < 0.01) n += megavolcano_function(GetFracDef(7), p) * n * 800.0f;
	else n += megavolcano_function(GetFracDef(7), p) * 8.0f;

	//n = (n > 0.0 ? n : 0.0); 
	//n = n*.1f;

	// BEWARE THE WALL OF TEXT
	if ((m_seed>>2) %3 > 2) {

		if (n < .2f) n += canyon3_ridged_function(GetFracDef(8), p) * n * 2;
		else if (n < .4f) n += canyon3_ridged_function(GetFracDef(8), p) * .4;
		else n += canyon3_ridged_function(GetFracDef(8), p) * (.4/n) * .4;

		if (n < .2f) n += canyon2_ridged_function(GetFracDef(8), p) * n * 2;
		else if (n < .4f) n += canyon2_ridged_function(GetFracDef(8), p) * .4;
		else n += canyon2_ridged_function(GetFracDef(8), p) * (.4/n) * .4;

		if (n < .2f) n += canyon_ridged_function(GetFracDef(8), p) * n * 2;
		else if (n < .4f) n += canyon_ridged_function(GetFracDef(8), p) * .4;
		else n += canyon_ridged_function(GetFracDef(8), p) * (.4/n) * .4;

		if (n < .2f) n += canyon3_ridged_function(GetFracDef(9), p) * n * 2;
		else if (n < .4f) n += canyon3_ridged_function(GetFracDef(9), p) * .4;
		else n += canyon3_ridged_function(GetFracDef(9), p) * (.4/n) * .4;

		if (n < .2f) n += canyon2_ridged_function(GetFracDef(9), p) * n * 2;
		else if (n < .4f) n += canyon2_ridged_function(GetFracDef(9), p) * .4;
		else n += canyon2_ridged_function(GetFracDef(9), p) * (.4/n) * .4;

		if (n < .2f) n += canyon_ridged_function(GetFracDef(9), p) * n * 2;
		else if (n < .4f) n += canyon_ridged_function(GetFracDef(9), p) * .4;
		else n += canyon_ridged_function(GetFracDef(9), p) * (.4/n) * .4;

	} else if ((m_seed>>2) %3 > 1) {

		if (n < .2f) n += canyon3_billow_function(GetFracDef(8), p) * n * 2;
		else if (n < .4f) n += canyon3_billow_function(GetFracDef(8), p) * .4;
		else n += canyon3_billow_function(GetFracDef(8), p) * (.4/n) * .4;

		if (n < .2f) n += canyon2_billow_function(GetFracDef(8), p) * n * 2;
		else if (n < .4f) n += canyon2_billow_function(GetFracDef(8), p) * .4;
		else n += canyon2_billow_function(GetFracDef(8), p) * (.4/n) * .4;

		if (n < .2f) n += canyon_billow_function(GetFracDef(8), p) * n * 2;
		else if (n < .4f) n += canyon_billow_function(GetFracDef(8), p) * .4;
		else n += canyon_billow_function(GetFracDef(8), p) * (.4/n) * .4;

		if (n < .2f) n += canyon3_billow_function(GetFracDef(9), p) * n * 2;
		else if (n < .4f) n += canyon3_billow_function(GetFracDef(9), p) * .4;
		else n += canyon3_billow_function(GetFracDef(9), p) * (.4/n) * .4;

		if (n < .2f) n += canyon2_billow_function(GetFracDef(9), p) * n * 2;
		else if (n < .4f) n += canyon2_billow_function(GetFracDef(9), p) * .4;
		else n += canyon2_billow_function(GetFracDef(9), p) * (.4/n) * .4;

		if (n < .2f) n += canyon_billow_function(GetFracDef(9), p) * n * 2;
		else if (n < .4f) n += canyon_billow_function(GetFracDef(9), p) * .4;
		else n += canyon_billow_function(GetFracDef(9), p) * (.4/n) * .4;

	} else {

		if (n < .2f) n += canyon3_voronoi_function(GetFracDef(8), p) * n * 2;
		else if (n < .4f) n += canyon3_voronoi_function(GetFracDef(8), p) * .4;
		else n += canyon3_voronoi_function(GetFracDef(8), p) * (.4/n) * .4;

		if (n < .2f) n += canyon2_voronoi_function(GetFracDef(8), p) * n * 2;
		else if (n < .4f) n += canyon2_voronoi_function(GetFracDef(8), p) * .4;
		else n += canyon2_voronoi_function(GetFracDef(8), p) * (.4/n) * .4;

		if (n < .2f) n += canyon_voronoi_function(GetFracDef(8), p) * n * 2;
		else if (n < .4f) n += canyon_voronoi_function(GetFracDef(8), p) * .4;
		else n += canyon_voronoi_function(GetFracDef(8), p) * (.4/n) * .4;

		if (n < .2f) n += canyon3_voronoi_function(GetFracDef(9), p) * n * 2;
		else if (n < .4f) n += canyon3_voronoi_function(GetFracDef(9), p) * .4;
		else n += canyon3_voronoi_function(GetFracDef(9), p) * (.4/n) * .4;

		if (n < .2f) n += canyon2_voronoi_function(GetFracDef(9), p) * n * 2;
		else if (n < .4f) n += canyon2_voronoi_function(GetFracDef(9), p) * .4;
		else n += canyon2_voronoi_function(GetFracDef(9), p) * (.4/n) * .4;

		if (n < .2f) n += canyon_voronoi_function(GetFracDef(9), p) * n * 2;
		else if (n < .4f) n += canyon_voronoi_function(GetFracDef(9), p) * .4;
		else n += canyon_voronoi_function(GetFracDef(9), p) * (.4/n) * .4;

	}

	n += -1.0f;
	n = (n > 0.0 ? n : 0.0); 

	n = n*.03f;

	//n += continents - (GetFracDef(0).amplitude*m_sealevel);

	if (n > 0.0) {
		// smooth in hills at shore edges
		if (n < 0.1) n += hills * n * 10.0f;
		else n += hills;
		if (n < 0.05) n += hills2 * n * 20.0f;
		else n += hills2 ;

		mountains  = octavenoise(GetFracDef(1), 0.5, p) *
			GetFracDef(2).amplitude * mountains*mountains*mountains;
		mountains2 = octavenoise(GetFracDef(4), 0.5, p) *
			GetFracDef(3).amplitude * mountains2*mountains2*mountains2;
		if (n > 0.5) n += mountains2 * (n - 0.5) ;
		if (n < 0.2) n += mountains * n * 5.0f ;
		else n += mountains  ; 
	}
	
	n = m_maxHeight*n;
	return (n > 0.0 ? n : 0.0); 
}
