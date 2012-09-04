#include "Terrain.h"
#include "TerrainNoise.h"

using namespace TerrainNoise;

template <>
const char *TerrainColorFractal<TerrainColorRock>::GetColorFractalName() const { return "Rock"; }

template <>
TerrainColorFractal<TerrainColorRock>::TerrainColorFractal(const SystemBody *body) : Terrain(body)
{
}

template <>
vector3d TerrainColorFractal<TerrainColorRock>::GetColor(const vector3d &p, double height, const vector3d &norm)
{
	double n = m_invMaxHeight*height/2;
	if (n <= 0) return m_darkrockColor[0];
	const double flatness = pow(p.Dot(norm), 20.0);
	const vector3d color_cliffs = m_rockColor[0];
	double equatorial_desert = (2.0-m_icyness)*(-1.0+2.0*octavenoise(4, 0.05, 2.0, (n*2.0)*p)) *
		1.0*(2.0-m_icyness)*(1.0-p.y*p.y);
	//double equatorial_region = octavenoise(GetFracDef(0), 0.54, p) * p.y * p.x;
	//double equatorial_region_2 = ridged_octavenoise(GetFracDef(1), 0.58, p) * p.x * p.x;
	// Below is to do with variable colours for different heights, it gives a nice effect.
	// n is height.
	vector3d col, tex1, tex2;
	col = interpolate_color(equatorial_desert, m_rockColor[2], m_darkrockColor[4]);
	//col = interpolate_color(equatorial_region, col, m_darkrockColor[4]);
	//col = interpolate_color(equatorial_region_2, m_rockColor[1], col);
	if (n > 0.9) {
		n -= 0.9; n *= 10.0;
		col = interpolate_color(n, m_rockColor[5], col );
		if (textures) {
			tex1 = interpolate_color(rock, col, color_cliffs);
			tex2 = interpolate_color(mud, col, color_cliffs);
			col = interpolate_color(flatness, tex1, tex2);
		} else col = interpolate_color(flatness, color_cliffs, col);
	return col;
	}
	else if (n > 0.8) {
		n -= 0.8; n *= 10.0;
		col = interpolate_color(n, col, m_rockColor[5]);
		if (textures) {
			tex1 = interpolate_color(rock, col, color_cliffs);
			tex2 = interpolate_color(mud, col, color_cliffs);
			col = interpolate_color(flatness, tex1, tex2);
		} else col = interpolate_color(flatness, color_cliffs, col);
	return col;
	}
	else if (n > 0.7) {
		n -= 0.7; n *= 10.0;
		col = interpolate_color(n, m_rockColor[4], col);
		if (textures) {
			tex1 = interpolate_color(rock, col, color_cliffs);
			tex2 = interpolate_color(mud, col, color_cliffs);
			col = interpolate_color(flatness, tex1, tex2);
		} else col = interpolate_color(flatness, color_cliffs, col);
	return col;
	}
	else if (n > 0.6) {
		n -= 0.6; n *= 10.0;
		col = interpolate_color(n, m_rockColor[1], m_rockColor[4]);
		if (textures) {
			tex1 = interpolate_color(rock2, col, color_cliffs);
			tex2 = interpolate_color(mud, col, m_rockColor[3]);
			col = interpolate_color(flatness, tex1, tex2);
		} else col = interpolate_color(flatness, color_cliffs, col);
	return col;
	}
	else if (n > 0.5) {
		n -= 0.5; n *= 10.0;
		col = interpolate_color(n, col, m_rockColor[1]);
		if (textures) {
			tex1 = interpolate_color(rock2, col, color_cliffs);
			tex2 = interpolate_color(rock, col, color_cliffs);
			col = interpolate_color(flatness, tex1, tex2);
		} else col = interpolate_color(flatness, color_cliffs, col);
	return col;
	}
	else if (n > 0.4) {
		n -= 0.4; n *= 10.0;
		col = interpolate_color(n, m_darkrockColor[3], col);
		if (textures) {
			tex1 = interpolate_color(rock, col, color_cliffs);
			tex2 = interpolate_color(mud, col, m_rockColor[3]);
			col = interpolate_color(flatness, tex1, tex2);
		} else col = interpolate_color(flatness, color_cliffs, col);
	return col;
	}
	if (n > 0.3) {
		n -= 0.3; n *= 10.0;
		col = interpolate_color(n, col, m_darkrockColor[3]);
		if (textures) {
			tex1 = interpolate_color(rock2, col, color_cliffs);
			tex2 = interpolate_color(mud, col, m_darkrockColor[6]);
			col = interpolate_color(flatness, tex1, tex2);
		} else col = interpolate_color(flatness, color_cliffs, col);
	return col;
	}
	else if (n > 0.2) {
		n -= 0.2; n *= 10.0;
		col = interpolate_color(n, m_rockColor[1], col);
		if (textures) {
			tex1 = interpolate_color(rock, col, color_cliffs);
			tex2 = interpolate_color(rock2, col, color_cliffs);
			col = interpolate_color(flatness, tex1, tex2);
		} else col = interpolate_color(flatness, color_cliffs, col);
	return col;
	}
	else if (n > 0.1) {
		n -= 0.1; n *= 10.0;
		col = interpolate_color(n, col, m_rockColor[1]);
		if (textures) {
			tex1 = interpolate_color(rock2, col, color_cliffs);
			tex2 = interpolate_color(mud, col, m_rockColor[3]);
			col = interpolate_color(flatness, tex1, tex2);
		} else col = interpolate_color(flatness, color_cliffs, col);
	return col;
	}
	else {
		n *= 10.0;
		col = interpolate_color(n, m_darkrockColor[0], col);
		if (textures) {
			tex1 = interpolate_color(rock, col, color_cliffs);
			tex2 = interpolate_color(mud, col, color_cliffs);
			col = interpolate_color(flatness, tex1, tex2);
		} else col = interpolate_color(flatness, color_cliffs, col);
	return col;
	}
}

