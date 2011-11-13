#ifndef _TERRAINFEATURE_H
#define _TERRAINFEATURE_H

#include "Terrain.h"

namespace TerrainFeature {

	double canyon_ridged_function(const fracdef_t &def, const vector3d &p);
	double canyon2_ridged_function(const fracdef_t &def, const vector3d &p);
	double canyon3_ridged_function(const fracdef_t &def, const vector3d &p);
	double canyon_normal_function(const fracdef_t &def, const vector3d &p);
	double canyon2_normal_function(const fracdef_t &def, const vector3d &p);
	double canyon3_normal_function(const fracdef_t &def, const vector3d &p);
	double canyon_voronoi_function(const fracdef_t &def, const vector3d &p);
	double canyon2_voronoi_function(const fracdef_t &def, const vector3d &p);
	double canyon3_voronoi_function(const fracdef_t &def, const vector3d &p);
	double canyon_billow_function(const fracdef_t &def, const vector3d &p);
	double canyon2_billow_function(const fracdef_t &def, const vector3d &p);
	double canyon3_billow_function(const fracdef_t &def, const vector3d &p);
	double crater_function(const fracdef_t &def, const vector3d &p);
	double impact_crater_function(const fracdef_t &def, const vector3d &p);
	double volcano_function(const fracdef_t &def, const vector3d &p);
	double megavolcano_function(const fracdef_t &def, const vector3d &p);
	double river_function(const fracdef_t &def, const vector3d &p, int style = 0);

};

#endif
