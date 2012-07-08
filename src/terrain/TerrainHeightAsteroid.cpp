#include "Terrain.h"
#include "TerrainNoise.h"
#include "TerrainFeature.h"

using namespace TerrainNoise;
using namespace TerrainFeature;

// Banded/Ridged pattern mountainous terrain, could pass for desert

template <>
const char *TerrainHeightFractal<TerrainHeightAsteroid>::GetHeightFractalName() const { return "Asteroid"; }

template <>
TerrainHeightFractal<TerrainHeightAsteroid>::TerrainHeightFractal(const SystemBody *body) : Terrain(body)
{
}

template <>
double TerrainHeightFractal<TerrainHeightAsteroid>::GetHeight(const vector3d &p)
{
	//p.x = 3*(p.y-p.x);
	//p.y = (-p.x*p.z) + (26.5*p.x) - p.y;
	//p.z = (p.x*p.y) - p.z;
	//float heightmap = octavenoise(64, 0.4, 1.6, 12.0*(3*(p.y-p.x), (-p.x*p.z) + (26.5*p.x) - p.y, (p.x*p.y) - p.z) );
	//Lorenz attractor:
	//float heightmap = octavenoise(8, 0.5, 2.0, (3*(p.y-p.x), (-p.x*p.z) + (26.5*p.x) - p.y, (p.x*p.y) - p.z) );
	//float heightmap = octavenoise(8, 0.2*octavenoise(1, 0.3, 3.7, (p.x*2.0-p.y, p.y*2.0-p.x, p.z)), 15.0*octavenoise(1, 0.5, 4.0, (p.x*2.0-p.y, p.y*2.0-p.x, p.z)), (p.x*2.0-p.y, p.y*2.0-p.x, p.z)) -
		//0.75*billow_octavenoise(8*octavenoise(1, 0.275, 3.2, (p.x*2.0-p.y, p.y*2.0-p.x, p.z)), 0.4*octavenoise(1, 0.4, 3.0, (p.x*2.0-p.y, p.y*2.0-p.x, p.z)), 4.0*octavenoise(1, 0.35, 3.7, (p.x*2.0-p.y, p.y*2.0-p.x, p.z)), (p.x*2.0-p.y, p.y*2.0-p.x, p.z));

	float heightmap = octavenoise(8, 0.4, 2.4, p);
	
	return m_maxHeight*heightmap;
}
