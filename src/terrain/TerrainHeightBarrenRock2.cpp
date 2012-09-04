#include "Terrain.h"
#include "TerrainNoise.h"
#include "TerrainFeature.h"

using namespace TerrainNoise;
using namespace TerrainFeature;

// Strange world, looks like its been hit by thousands of years of erosion. 
// Could be used as a basis for terrains that should have erosion.

template <>
const char *TerrainHeightFractal<TerrainHeightBarrenRock2>::GetHeightFractalName() const { return "Barren Rock 2"; }

template <>
TerrainHeightFractal<TerrainHeightBarrenRock2>::TerrainHeightFractal(const SystemBody *body) : Terrain(body)
{
}

template <>
double TerrainHeightFractal<TerrainHeightBarrenRock2>::GetHeight(const vector3d &p)
{

	float heightmap = billow_octavenoise(16, 0.3*octavenoise(8, 0.4, 2.5, p),Clamp(5.0*ridged_octavenoise(8, 0.377, 4.0, p), 1.0, 5.0), p);

	return m_maxHeight*heightmap;
}
