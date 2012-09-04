#include "Terrain.h"
#include "TerrainNoise.h"
#include "TerrainFeature.h"

using namespace TerrainNoise;
using namespace TerrainFeature;

// Craggy/Spikey terrain with massive canyons

template <>
const char *TerrainHeightFractal<TerrainHeightBarrenRock3>::GetHeightFractalName() const { return "Barren Rock 3"; }

template <>
TerrainHeightFractal<TerrainHeightBarrenRock3>::TerrainHeightFractal(const SystemBody *body) : Terrain(body)
{
}

template <>
double TerrainHeightFractal<TerrainHeightBarrenRock3>::GetHeight(const vector3d &p)
{

	float heightmap = 0.07*voronoiscam_octavenoise(12, Clamp(fabs(0.165 - (0.38*river_octavenoise(12, 0.4, 2.5, p))), 0.15, 0.5),Clamp(8.0*billow_octavenoise(12, 0.37, 4.0, p), 0.5, 9.0), p);

	return m_maxHeight*heightmap;
}
