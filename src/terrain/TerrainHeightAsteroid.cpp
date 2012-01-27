#include "Terrain.h"
#include "TerrainNoise.h"
#include "TerrainFeature.h"

using namespace TerrainNoise;
using namespace TerrainFeature;

// Banded/Ridged pattern mountainous terrain, could pass for desert

template <>
const char *TerrainHeightFractal<TerrainHeightAsteroid>::GetHeightFractalName() const { return "Asteroid"; }

template <>
TerrainHeightFractal<TerrainHeightAsteroid>::TerrainHeightFractal(const SBody *body) : Terrain(body)
{
}

template <>
double TerrainHeightFractal<TerrainHeightAsteroid>::GetHeight(const vector3d &p)
{
	float heightmap = octavenoise(8, 0.2*octavenoise(4, 0.3, 3.7, p), 15.0*octavenoise(1, 0.5, 4.0, p), p) - 
		0.75*billow_octavenoise(16*octavenoise(3, 0.275, 3.2, p), 0.4*octavenoise(2, 0.4, 3.0, p), 4.0*octavenoise(1, 0.35, 3.7, p), p);

	return m_maxHeight*heightmap;
}
