#include "Terrain.h"
#include "TerrainNoise.h"
#include "TerrainFeature.h"

using namespace TerrainNoise;
using namespace TerrainFeature;

template <>
const char *TerrainHeightFractal<TerrainHeightAsteroid>::GetHeightFractalName() const { return "Asteroid"; }

template <>
TerrainHeightFractal<TerrainHeightAsteroid>::TerrainHeightFractal(const SystemBody *body) : Terrain(body)
{
	SetFracDef(0, m_maxHeightInMeters, m_planetRadius);
	// craters
	SetFracDef(1, 5000.0, 1000000.0, 1000.0*m_fracmult);
}

template <>
double TerrainHeightFractal<TerrainHeightAsteroid>::GetHeight(const vector3d &p)
{
	return std::max(0.0, m_maxHeight * (octavenoise(GetFracDef(0), 0.5, p) + 
			GetFracDef(1).amplitude * crater_function(GetFracDef(1), p)));
}
