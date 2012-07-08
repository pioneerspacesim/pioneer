#include "Terrain.h"
#include "TerrainNoise.h"
#include "TerrainFeature.h"

using namespace TerrainNoise;
using namespace TerrainFeature;

// Cool terrain for asteroids or small planets

template <>
const char *TerrainHeightFractal<TerrainHeightAsteroid4>::GetHeightFractalName() const { return "Asteroid"; }

template <>
TerrainHeightFractal<TerrainHeightAsteroid4>::TerrainHeightFractal(const SystemBody *body) : Terrain(body)
{
}

template <>
double TerrainHeightFractal<TerrainHeightAsteroid4>::GetHeight(const vector3d &p)
{
	float heightmap = octavenoise(6, 0.2*octavenoise(2, 0.3, 3.7, p), 2.8*ridged_octavenoise(3, 0.5, 3.0, p), p) * 
		0.75*ridged_octavenoise(16*octavenoise(3, 0.275, 2.9, p), 0.3*octavenoise(2, 0.4, 3.0, p), 2.8*ridged_octavenoise(8, 0.35, 2.7, p), p);

	return m_maxHeight*heightmap;
}
