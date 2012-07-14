#include "Terrain.h"
#include "TerrainNoise.h"

using namespace TerrainNoise;

template <>
const char *TerrainColorFractal<TerrainColorBandedRock>::GetColorFractalName() const { return "BandedRock"; }

template <>
TerrainColorFractal<TerrainColorBandedRock>::TerrainColorFractal(const SystemBody *body) : Terrain(body)
{
}

template <>
vector3d TerrainColorFractal<TerrainColorBandedRock>::GetColor(const vector3d &p, double height, const vector3d &norm)
{
	const double flatness = pow(p.Dot(norm), 6.0);
	double n = fabs(noise(vector3d(height*10000.0,0.0,0.0)));
	vector3d col = interpolate_color(n, m_rockColor[0], m_rockColor[1]);
	return interpolate_color(flatness, col, m_rockColor[2]);
}

