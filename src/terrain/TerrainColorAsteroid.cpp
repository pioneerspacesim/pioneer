#include "Terrain.h"
#include "TerrainNoise.h"

using namespace TerrainNoise;

template <>
const char *TerrainColorFractal<TerrainColorAsteroid>::GetColorFractalName() const { return "Asteroid"; }

template <>
TerrainColorFractal<TerrainColorAsteroid>::TerrainColorFractal(const SystemBody *body) : Terrain(body)
{
}

template <>
vector3d TerrainColorFractal<TerrainColorAsteroid>::GetColor(const vector3d &p, double height, const vector3d &norm)
{
	double n = m_invMaxHeight*height/2;

	if (n <= 0.02) {
		const double flatness = pow(p.Dot(norm), 6.0);
		const vector3d color_cliffs = m_rockColor[1];

		double equatorial_desert = (2.0)*(-1.0+2.0*octavenoise(12, 0.5, 2.0, (n*2.0)*p)) *
			1.0*(2.0)*(1.0-p.y*p.y);

		vector3d col;
		col = interpolate_color(equatorial_desert, m_rockColor[0], m_greyrockColor[3]);
		col = interpolate_color(n, col, vector3d(1.5,1.35,1.3));
		col = interpolate_color(flatness, color_cliffs, col);
		return col;
	} else {
		const double flatness = pow(p.Dot(norm), 6.0);
		const vector3d color_cliffs = m_greyrockColor[1];

		double equatorial_desert = (2.0)*(-1.0+2.0*octavenoise(12, 0.5, 2.0, (n*2.0)*p)) *
			1.0*(2.0)*(1.0-p.y*p.y);

		vector3d col;
		col = interpolate_color(equatorial_desert, m_greyrockColor[0], m_greyrockColor[2]);
		col = interpolate_color(n, col, m_rockColor[3]);
		col = interpolate_color(flatness, color_cliffs, col);
		return col;
	}
}

