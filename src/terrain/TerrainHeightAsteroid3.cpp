#include "Terrain.h"
#include "TerrainNoise.h"
#include "TerrainFeature.h"

using namespace TerrainNoise;
using namespace TerrainFeature;

// Banded/Ridged pattern mountainous terrain, could pass for desert

template <>
const char *TerrainHeightFractal<TerrainHeightAsteroid3>::GetHeightFractalName() const { return "Asteroid"; }

template <>
TerrainHeightFractal<TerrainHeightAsteroid3>::TerrainHeightFractal(const SystemBody *body) : Terrain(body)
{
}

template <>
double TerrainHeightFractal<TerrainHeightAsteroid3>::GetHeight(const vector3d &p)
{
	double n = octavenoise(8, 0.5, 4.0, p) * ridged_octavenoise(8, 0.5, 4.0, p);
	n *= m_maxHeight;
	ApplySimpleHeightRegions(n, p);
	return (n > 0.0) ? n : 0.0;
}
