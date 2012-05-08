#include "Terrain.h"
#include "TerrainNoise.h"

using namespace TerrainNoise;

template <>
const char *TerrainColorFractal<TerrainColorRock2>::GetColorFractalName() const { return "Rock2"; }

template <>
TerrainColorFractal<TerrainColorRock2>::TerrainColorFractal(const SystemBody *body) : Terrain(body)
{
}

template <>
vector3d TerrainColorFractal<TerrainColorRock2>::GetColor(const vector3d &p, double height, const vector3d &norm)
{
	double n = m_invMaxHeight*height/2;

	if (n <= 0) return m_greyrockColor[1];		

	const double flatness = pow(p.Dot(norm), 6.0);
	const vector3d color_cliffs = m_greyrockColor[1];

	double equatorial_desert = (2.0-m_icyness)*(-1.0+2.0*octavenoise(12, 0.5, 2.0, (n*2.0)*p)) *
			1.0*(2.0-m_icyness)*(1.0-p.y*p.y);


	// Below is to do with variable colours for different heights, it gives a nice effect.
	// n is height.
	vector3d col;
	col = interpolate_color(equatorial_desert, m_greyrockColor[2], m_greyrockColor[3]);
	if (n > 0.45) {
	col = interpolate_color(n, col, m_greyrockColor[6]);
	col = interpolate_color(flatness, color_cliffs, col);
	return col;
	}
	else if (n > 0.4) {
	col = interpolate_color(n, col, m_greyrockColor[5]);
	col = interpolate_color(flatness, color_cliffs, col);
	return col;
	}
	else if (n > 0.35) {
	col = interpolate_color(n, m_greyrockColor[7], col);
	col = interpolate_color(flatness, color_cliffs, col);
	return col;
	}
	else if (n > 0.3) {
	col = interpolate_color(n, m_greyrockColor[0], col);
	col = interpolate_color(flatness, color_cliffs, col);
	return col;
	}
	else if (n > 0.25) {
	col = interpolate_color(n, col, m_greyrockColor[0]);
	col = interpolate_color(flatness, color_cliffs, col);
	return col;
	}
	else if (n > 0.2) {
	col = interpolate_color(n, m_greyrockColor[2], col);
	col = interpolate_color(flatness, color_cliffs, col);
	return col;
	}
	else if (n > 0.15) {
	col = interpolate_color(n, m_greyrockColor[3], col);
	col = interpolate_color(flatness, color_cliffs, col);
	return col;
	}
	else if (n > 0.1) {
	col = interpolate_color(n, col, m_greyrockColor[3]);
	col = interpolate_color(flatness, color_cliffs, col);
	return col;
	}
	else if (n > 0.05) {
	col = interpolate_color(n, col, m_greyrockColor[1]);
	col = interpolate_color(flatness, color_cliffs, col);
	return col;
	}
	else {
	col = interpolate_color(n, m_greyrockColor[0], col);
	col = interpolate_color(flatness, color_cliffs, col);
	return col;
	}
}

