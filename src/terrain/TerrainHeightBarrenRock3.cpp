#include "Terrain.h"
#include "TerrainNoise.h"
#include "TerrainFeature.h"

using namespace TerrainNoise;
using namespace TerrainFeature;

// Craggy/Spikey terrain with massive canyons

template <>
const char *TerrainHeightFractal<TerrainHeightBarrenRock3>::GetHeightFractalName() const { return "Barren Rock 3"; }

template <>
TerrainHeightFractal<TerrainHeightBarrenRock3>::TerrainHeightFractal(const SBody *body) : Terrain(body)
{
}

template <>
double TerrainHeightFractal<TerrainHeightBarrenRock3>::GetHeight(const vector3d &p)
{

	float heightmap = voronoiscam_octavenoise(12, 0.3*river_octavenoise(12, 0.4, 2.5, p),Clamp(8.0*billow_octavenoise(12, 0.37, 4.0, p), 0.5, 9.0), p);

	return m_maxHeight*heightmap;
}
