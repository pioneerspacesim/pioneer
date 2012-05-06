#include "Terrain.h"
#include "TerrainNoise.h"

using namespace TerrainNoise;

template <>
const char *TerrainColorFractal<TerrainColorDesert>::GetColorFractalName() const { return "Desert"; }

template <>
TerrainColorFractal<TerrainColorDesert>::TerrainColorFractal(const SystemBody *body) : Terrain(body)
{
}

template <>
vector3d TerrainColorFractal<TerrainColorDesert>::GetColor(const vector3d &p, double height, const vector3d &norm)
{
	double n = m_invMaxHeight*height/2;
	const double flatness = pow(p.Dot(norm), 6.0);
	const vector3d color_cliffs = m_rockColor[1];
	// Ice has been left as is so the occasional desert world will have polar ice-caps like mars
	if (fabs(m_icyness*p.y) + m_icyness*n > 1) {
		return interpolate_color(flatness, color_cliffs, vector3d(1,1,1));
	}
	double equatorial_desert = (2.0-m_icyness)*(-1.0+2.0*octavenoise(12, 0.5, 2.0, (n*2.0)*p)) *
			1.0*(2.0-m_icyness)*(1.0-p.y*p.y);
	vector3d col;
	if (n > .4) {
		n = n*n;
		col = interpolate_color(equatorial_desert, vector3d(.8,.75,.5), vector3d(.52, .5, .3));
		col = interpolate_color(n, col, vector3d(.1, .0, .0));
		col = interpolate_color(flatness, color_cliffs, col);
		return col;
	} else if (n > .3) {
		n = n*n;
		col = interpolate_color(equatorial_desert, vector3d(.81, .68, .3), vector3d(.85, .7, 0));
		col = interpolate_color(n, col, vector3d(-1.2,-.84,.35));
		col = interpolate_color(flatness, color_cliffs, col);
		return col;
	} else if (n > .2) {
		col = interpolate_color(equatorial_desert, vector3d(-0.4, -0.47, -0.6), vector3d(-.6, -.7, -2));
		col = interpolate_color(n, col, vector3d(4, 3.95, 3.94));
		col = interpolate_color(flatness, color_cliffs, col);
		return col;
	} else {
		col = interpolate_color(equatorial_desert, vector3d(.78, .73, .68), vector3d(.8, .77, .5));
		col = interpolate_color(n, col, vector3d(-2.0, -2.3, -2.4));
		col = interpolate_color(flatness, color_cliffs, col);
		return col;
	}	
}

