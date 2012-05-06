#include "Terrain.h"
#include "TerrainNoise.h"

using namespace TerrainNoise;

template <>
const char *TerrainColorFractal<TerrainColorDeadWithWater>::GetColorFractalName() const { return "DeadWithWater"; }

template <>
TerrainColorFractal<TerrainColorDeadWithWater>::TerrainColorFractal(const SystemBody *body) : Terrain(body)
{
}

template <>
vector3d TerrainColorFractal<TerrainColorDeadWithWater>::GetColor(const vector3d &p, double height, const vector3d &norm)
{
	double n = m_invMaxHeight*height;
	if (n <= 0) return vector3d(0.0,0.0,0.5);
	else return interpolate_color(n, vector3d(.2,.2,.2), vector3d(.6,.6,.6));
}

