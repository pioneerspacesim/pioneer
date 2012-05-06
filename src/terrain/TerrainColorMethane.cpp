#include "Terrain.h"
#include "TerrainNoise.h"

using namespace TerrainNoise;

template <>
const char *TerrainColorFractal<TerrainColorMethane>::GetColorFractalName() const { return "Methane"; }

template <>
TerrainColorFractal<TerrainColorMethane>::TerrainColorFractal(const SystemBody *body) : Terrain(body)
{
}

template <>
vector3d TerrainColorFractal<TerrainColorMethane>::GetColor(const vector3d &p, double height, const vector3d &norm)
{
	double n = m_invMaxHeight*height;
	if (n <= 0) return vector3d(.3,.0,.0);
	else return interpolate_color(n, vector3d(.3,.2,.0), vector3d(.6,.3,.0));
}

