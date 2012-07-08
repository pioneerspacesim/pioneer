#include "Terrain.h"
#include "TerrainNoise.h"
#include "TerrainFeature.h"

using namespace TerrainNoise;
using namespace TerrainFeature;

// Banded/Ridged pattern mountainous terrain, could pass for desert

template <>
const char *TerrainHeightFractal<TerrainHeightAsteroid2>::GetHeightFractalName() const { return "Asteroid"; }

template <>
TerrainHeightFractal<TerrainHeightAsteroid2>::TerrainHeightFractal(const SystemBody *body) : Terrain(body)
{
}

template <>
double TerrainHeightFractal<TerrainHeightAsteroid2>::GetHeight(const vector3d &p)
{
	float heightmap = voronoiscam_octavenoise(6, 0.2*octavenoise(2, 0.3, 3.7, p), 15.0*octavenoise(2, 0.5, 4.0, p), p) * 
		0.75*ridged_octavenoise(16*octavenoise(2, 0.275, 3.2, p), 0.4*ridged_octavenoise(4, 0.4, 3.0, p), 4.0*octavenoise(3, 0.35, 3.7, p), p);

	return m_maxHeight*heightmap;
}
