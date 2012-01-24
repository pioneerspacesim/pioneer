#include "Terrain.h"
#include "TerrainNoise.h"

using namespace TerrainNoise;

template <>
const char *TerrainColorFractal<TerrainColorRock>::GetColorFractalName() const { return "Rock"; }

template <>
TerrainColorFractal<TerrainColorRock>::TerrainColorFractal(const SBody *body) : Terrain(body)
{
}

template <>
vector3d TerrainColorFractal<TerrainColorRock>::GetColor(const vector3d &p, double height, const vector3d &norm)
{
	float n = m_invMaxHeight*height/2;
	const vector3d color_cliffs = m_rockColor[0];
	if (n <= 0) return color_cliffs;		
	const float flatness = pow(p.Dot(norm), 6.0);
	float noise_variation = octavenoise(2, 0.11, 9.25, p*p.y);
	// Below is to do with variable colours for different heights, it gives a nice effect.
	// n is height.
	vector3d col;
	col = interpolate_color(noise_variation, m_rockColor[2], m_rockColor[3]);
	if (n > 0.9) {
		n -= 0.9; n *= 10.0;
		col = interpolate_color(n, m_rockColor[5], col );
		col = interpolate_color(flatness, color_cliffs, col);
	return col;
	}
	else if (n > 0.8) {
		n -= 0.8; n *= 10.0;
		col = interpolate_color(n, m_rockColor[1], m_rockColor[5]);
		col = interpolate_color(flatness, color_cliffs, col);
	return col;
	}
	else if (n > 0.7) {
		n -= 0.7; n *= 10.0;
		col = interpolate_color(n, m_rockColor[4], m_rockColor[1]);
		col = interpolate_color(flatness, color_cliffs, col);
	return col;
	}
	else if (n > 0.6) {
		n -= 0.6; n *= 10.0;
		col = interpolate_color(n, m_rockColor[0], m_rockColor[4]);
		col = interpolate_color(flatness, color_cliffs, col);
	return col;
	}
	else if (n > 0.5) {
		n -= 0.5; n *= 10.0;
		col = interpolate_color(n, col, m_rockColor[0]);
		col = interpolate_color(flatness, color_cliffs, col);
	return col;
	}
	else if (n > 0.4) {
		n -= 0.4; n *= 10.0;
		col = interpolate_color(n, m_darkrockColor[3], col);
		col = interpolate_color(flatness, color_cliffs, col);
	return col;
	}
	if (n > 0.3) {
		n -= 0.3; n *= 10.0;
		col = interpolate_color(n, col, m_darkrockColor[3]);
		col = interpolate_color(flatness, color_cliffs, col);
	return col;
	}
	else if (n > 0.2) {
		n -= 0.2; n *= 10.0;
		col = interpolate_color(n, m_darkrockColor[1], col);
		col = interpolate_color(flatness, color_cliffs, col);
	return col;
	}
	else if (n > 0.1) {
		n -= 0.1; n *= 10.0;
		col = interpolate_color(n, col, m_darkrockColor[1]);
		col = interpolate_color(flatness, color_cliffs, col);
	return col;
	}
	else {
		n *= 10.0;
		col = interpolate_color(n, m_rockColor[0], col);
		col = interpolate_color(flatness, color_cliffs, col);
	return col;
	}
}

