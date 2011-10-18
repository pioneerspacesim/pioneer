#include "Terrain.h"
#include "TerrainNoise.h"
#include "TerrainComponent.h"

using namespace TerrainNoise;
using namespace TerrainComponent;

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

template <>
void TerrainHeightFractal<TerrainHeightMountainsRiversVolcano>::InitFracDef(MTRand &rand)
{
}
