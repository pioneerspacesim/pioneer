#include "Terrain.h"
#include "TerrainNoise.h"

using namespace TerrainNoise;

template <>
double TerrainHeightFractal<TerrainHeightMountainsRidged>::GetHeight(const vector3d &p)
{
	double continents = octavenoise(GetFracDef(0), 0.5, p) - m_sealevel;
	if (continents < 0) return 0;
	// unused variable \\ double mountain_distrib = octavenoise(GetFracDef(1), 0.5, p);
	double mountains = octavenoise(GetFracDef(2), 0.5, p);
	double mountains2 = ridged_octavenoise(GetFracDef(3), 0.5, p);

	double hill_distrib = octavenoise(GetFracDef(4), 0.5, p);
	double hills = hill_distrib * GetFracDef(5).amplitude * ridged_octavenoise(GetFracDef(5), 0.5, p);
	double hills2 = hill_distrib * GetFracDef(6).amplitude * octavenoise(GetFracDef(6), 0.5, p);

	double hill2_distrib = octavenoise(GetFracDef(7), 0.5, p);
	double hills3 = hill2_distrib * GetFracDef(8).amplitude * ridged_octavenoise(GetFracDef(8), 0.5, p);
	double hills4 = hill2_distrib * GetFracDef(9).amplitude * ridged_octavenoise(GetFracDef(9), 0.5, p);

	double n = continents - (GetFracDef(0).amplitude*m_sealevel);

	if (n > 0.0) {
		// smooth in hills at shore edges
		if (n < 0.1) n += hills * n * 10.0f;
		else n += hills;
		if (n < 0.05) n += hills2 * n * 20.0f;
		else n += hills2 ;

		if (n < 0.1) n += hills3 * n * 10.0f;
		else n += hills3;
		if (n < 0.05) n += hills4 * n * 20.0f;
		else n += hills4 ;

		mountains  = octavenoise(GetFracDef(1), 0.5, p) *
			GetFracDef(2).amplitude * mountains*mountains*mountains;
		mountains2 = octavenoise(GetFracDef(4), 0.5, p) *
			GetFracDef(3).amplitude * mountains2*mountains2*mountains2*mountains2;
		if (n > 0.2) n += mountains2 * (n - 0.2) ;
		if (n < 0.2) n += mountains * n * 5.0f ;
		else n += mountains  ; 
	}
	
	n = m_maxHeight*n;
	return (n > 0.0 ? n : 0.0); 
}
