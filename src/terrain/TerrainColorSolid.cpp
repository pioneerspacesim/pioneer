#include "Terrain.h"

template <>
const char *TerrainColorFractal<TerrainColorSolid>::GetColorFractalName() const { return "Solid"; }

template <>
TerrainColorFractal<TerrainColorSolid>::TerrainColorFractal(const SystemBody *body) : Terrain(body)
{
}

template <>
vector3d TerrainColorFractal<TerrainColorSolid>::GetColor(const vector3d &p, double height, const vector3d &norm)
{
	return vector3d(1.0);
}
