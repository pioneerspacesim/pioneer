#include "Terrain.h"
#include "TerrainNoise.h"

using namespace TerrainNoise;

template <>
const char *TerrainColorFractal<TerrainColorTFGood>::GetColorFractalName() const { return "TFGood"; }

template <>
TerrainColorFractal<TerrainColorTFGood>::TerrainColorFractal(const SystemBody *body) : Terrain(body)
{
}

template <>
vector3d TerrainColorFractal<TerrainColorTFGood>::GetColor(const vector3d &p, double height, const vector3d &norm)
{
	double n = m_invMaxHeight*height;
	const double flatness = pow(p.Dot(norm), 8.0);
	vector3d color_cliffs = m_rockColor[5];
	// ice on mountains and poles
		if (fabs(m_icyness*p.y) + m_icyness*n > 1) {
			return interpolate_color(flatness, color_cliffs, vector3d(1,1,1));
		}

	double equatorial_desert = (2.0-m_icyness)*(-1.0+2.0*octavenoise(12, 0.5, 2.0, (n*2.0)*p)) *
			1.0*(2.0-m_icyness)*(1.0-p.y*p.y);
	// This is for fake ocean depth by the coast.
	double continents = octavenoise(GetFracDef(0), 0.7*
				ridged_octavenoise(GetFracDef(8), 0.58, p), p) - m_sealevel*0.6;

	vector3d col;
	//we don't want water on the poles if there are ice-caps
	if (fabs(m_icyness*p.y) > 0.75) {
		col = interpolate_color(equatorial_desert, vector3d(0.42, 0.46, 0), vector3d(0.5, 0.3, 0));
		col = interpolate_color(flatness, col, vector3d(1,1,1));
		return col;
	}
	// water
	if (n <= 0) {
			// Oooh, pretty coastal regions with shading based on underwater depth.
		n += continents - (GetFracDef(0).amplitude*m_sealevel*0.49);
		n *= 10.0;
		n = (n>0.3 ? 0.3-(n*n*n-0.027) : n);
		col = interpolate_color(equatorial_desert, vector3d(0,0,0.15), vector3d(0,0,0.25));
		col = interpolate_color(n, col, vector3d(0,0.8,0.6));
		return col;
	}

	// More sensitive height detection for application of colours
	
	if (n > 0.5) {
	col = interpolate_color(equatorial_desert, m_rockColor[2], m_rockColor[4]);
	col = interpolate_color(n, col, m_darkrockColor[6]);
	col = interpolate_color(flatness, color_cliffs, col);
	return col;
	}
	else if (n > 0.25) { 
	color_cliffs = m_darkrockColor[1];
	col = interpolate_color(equatorial_desert, m_darkrockColor[5], m_darkrockColor[7]);
	col = interpolate_color(n, col, m_rockColor[1]);
	col = interpolate_color(flatness, color_cliffs, col);
	return col;
	}
	else if (n > 0.05) {  
	col = interpolate_color(equatorial_desert, m_darkrockColor[5], m_darkrockColor[7]);
	color_cliffs = col;
	col = interpolate_color(equatorial_desert, vector3d(.45,.43, .2), vector3d(.4, .43, .2));
	col = interpolate_color(n, col, vector3d(-1.66,-2.3, -1.75));
	col = interpolate_color(flatness, color_cliffs, col);
	return col;
	}
	else if (n > 0.01) { 
	color_cliffs = vector3d(0.2,0.28,0.2);
	col = interpolate_color(equatorial_desert, vector3d(.15,.5, -.1), vector3d(.2, .6, -.1));
	col = interpolate_color(n, col, vector3d(5,-5, 5));
	col = interpolate_color(flatness, color_cliffs, col);
	return col;
	}
	else if (n > 0.005) {   
	color_cliffs = vector3d(0.25,0.28,0.2);
	col = interpolate_color(equatorial_desert, vector3d(.45,.6,0), vector3d(.5, .6, .0));
	col = interpolate_color(n, col, vector3d(-10,-10,0));
	col = interpolate_color(flatness, color_cliffs, col);
	return col;
	}
	else { 
	color_cliffs = vector3d(0.3,0.1,0.0);
	col = interpolate_color(equatorial_desert, vector3d(.35,.3,0), vector3d(.4, .3, .0));
	col = interpolate_color(n, col, vector3d(0,20,0));
	col = interpolate_color(flatness, color_cliffs, col);
	return col;
	}
}

