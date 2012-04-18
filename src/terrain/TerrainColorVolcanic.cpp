#include "Terrain.h"
#include "TerrainNoise.h"

using namespace TerrainNoise;

template <>
const char *TerrainColorFractal<TerrainColorVolcanic>::GetColorFractalName() const { return "Volcanic"; }

template <>
TerrainColorFractal<TerrainColorVolcanic>::TerrainColorFractal(const SystemBody *body) : Terrain(body)
{
}

template <>
vector3d TerrainColorFractal<TerrainColorVolcanic>::GetColor(const vector3d &p, double height, const vector3d &norm)
{
	double n = m_invMaxHeight*height;
	const double flatness = pow(p.Dot(norm), 6.0);
	const vector3d color_cliffs = m_rockColor[2];		
	double equatorial_desert = (-1.0+2.0*octavenoise(12, 0.5, 2.0, (n*2.0)*p)) *
			1.0*(1.0-p.y*p.y);

	vector3d col;

	if (n > 0.4){
	col = interpolate_color(equatorial_desert, vector3d(.3,.2,0), vector3d(.3, .1, .0));
	col = interpolate_color(n, col, vector3d(.1, .0, .0));
	col = interpolate_color(flatness, color_cliffs, col);
	} else if (n > 0.2){
	col = interpolate_color(equatorial_desert, vector3d(1.2,1,0), vector3d(.9, .3, .0));
	col = interpolate_color(n, col, vector3d(-1.1, -1, .0));
	col = interpolate_color(flatness, color_cliffs, col);
	} else if (n > 0.1){
	col = interpolate_color(equatorial_desert, vector3d(.2,.1,0), vector3d(.1, .05, .0));
	col = interpolate_color(n, col, vector3d(2.5, 2, .0));
	col = interpolate_color(flatness, color_cliffs, col);
	} else {
	col = interpolate_color(equatorial_desert, vector3d(.75,.6,0), vector3d(.75, .2, .0));
	col = interpolate_color(n, col, vector3d(-2, -2.2, .0));
	col = interpolate_color(flatness, color_cliffs, col);
	}
	return col;
}

