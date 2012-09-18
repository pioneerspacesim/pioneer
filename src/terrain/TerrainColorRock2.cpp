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
	if (n <= 0) return m_darkrockColor[0];
	const double flatness = pow(p.Dot(norm), 6.0);
	const vector3d color_cliffs = m_rockColor[0];
	double equatorial_desert = (2.0-m_icyness)*(-1.0+2.0*octavenoise(4, 0.05, 2.0, (n*2.0)*p)) *
		1.0*(2.0-m_icyness)*(1.0-p.y*p.y);
	//double equatorial_region = octavenoise(GetFracDef(0), 0.54, p) * p.y * p.x;
	//double equatorial_region_2 = ridged_octavenoise(GetFracDef(1), 0.58, p) * p.x * p.x;
	// Below is to do with variable colours for different heights, it gives a nice effect.
	// n is height.
	vector3d col;
	col = interpolate_color(equatorial_desert, m_rockColor[2], m_darkrockColor[4]);
	//col = interpolate_color(equatorial_region, col, m_darkrockColor[4]);
	//col = interpolate_color(equatorial_region_2, m_rockColor[1], col);
	if (n > 0.9) {
		n -= 0.9; n *= 10.0;
		col = interpolate_color(n, m_rockColor[5], col );
		col = interpolate_color(flatness, color_cliffs, col);
	return col;
	}
	else if (n > 0.8) {
		n -= 0.8; n *= 10.0;
		col = interpolate_color(n, col, m_rockColor[5]);
		col = interpolate_color(flatness, color_cliffs, col);
	return col;
	}
	else if (n > 0.7) {
		n -= 0.7; n *= 10.0;
		col = interpolate_color(n, m_rockColor[4], col);
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
		col = interpolate_color(n, m_rockColor[1], col);
		col = interpolate_color(flatness, color_cliffs, col);
	return col;
	}
	else if (n > 0.1) {
		n -= 0.1; n *= 10.0;
		col = interpolate_color(n, col, m_rockColor[1]);
		col = interpolate_color(flatness, color_cliffs, col);
	return col;
	}
	else {
		n *= 10.0;
		col = interpolate_color(n, m_darkrockColor[0], col);
		col = interpolate_color(flatness, color_cliffs, col);
	return col;
	}
}

