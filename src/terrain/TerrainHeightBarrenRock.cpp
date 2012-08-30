#include "Terrain.h"
#include "TerrainNoise.h"
#include "TerrainFeature.h"

using namespace TerrainNoise;
using namespace TerrainFeature;

// Banded/Ridged pattern mountainous terrain, could pass for desert

template <>
const char *TerrainHeightFractal<TerrainHeightBarrenRock>::GetHeightFractalName() const { return "Barren Rock"; }

template <>
TerrainHeightFractal<TerrainHeightBarrenRock>::TerrainHeightFractal(const SystemBody *body) : Terrain(body)
{
	//SetFracDef(0, m_maxHeightInMeters, m_planetRadius);
	// craters
	//SetFracDef(1, 5000.0, 1000000.0, 10000.0*m_fracmult);
}

template <>
double TerrainHeightFractal<TerrainHeightBarrenRock>::GetHeight(const vector3d &p)
{
	/*return std::max(0.0, m_maxHeight * (octavenoise(GetFracDef(0), 0.5, p) + 
			GetFracDef(1).amplitude * crater_function(GetFracDef(1), p)));*/
			//fuck the fracdefs, direct control is better:
	float heightmap = ridged_octavenoise(16, 0.5*octavenoise(8, 0.4, 2.5, p),Clamp(5.0*octavenoise(8, 0.257, 4.0, p), 1.0, 5.0), p);

	return m_maxHeight*2.0*heightmap*heightmap;
}
