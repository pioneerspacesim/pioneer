// Copyright © 2008-2013 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#include "Terrain.h"
#include "TerrainNoise.h"
#include "TerrainFeature.h"

using namespace TerrainNoise;
using namespace TerrainFeature;

// Cool terrain for asteroids or small planets

template <>
const char *TerrainHeightFractal<TerrainHeightAsteroid4>::GetHeightFractalName() const { return "Asteroid4"; }

/*static double s_heightScale = 0.05;
static double s_widthScale = 20.0;
static double s_fracMultScale = 10000.0;
#pragma optimize("",off)*/
template <>
TerrainHeightFractal<TerrainHeightAsteroid4>::TerrainHeightFractal(const SystemBody *body) : Terrain(body)
{
	double height = m_maxHeightInMeters*m_rand.Double(0.03,0.07);
	double width = m_maxHeightInMeters*m_rand.Double(15.3,60.2);
	double fracMultScale = m_rand.Double(9351.0, 10731);
	for( int i=0; i<3 ; i++ ) {
		SetFracDef(i, height, width, fracMultScale*m_fracmult);
		height *= m_rand.Double(0.4,0.6);	// gets bigger?
		width -= m_rand.Double(width*0.04,width*0.06);	// gets smaller
		fracMultScale *= m_rand.Double(0.77,0.93);
	}
	/*const double height = m_maxHeightInMeters*s_heightScale;
	const double width = m_maxHeightInMeters*s_widthScale;
	const double fracMultScale = s_fracMultScale;
	SetFracDef(0, height, width, fracMultScale*m_fracmult);*/
}

template <>
double TerrainHeightFractal<TerrainHeightAsteroid4>::GetHeight(const vector3d &p) const
{
	double n = octavenoise(6, 0.2*octavenoise(2, 0.3, 3.7, p), 2.8*ridged_octavenoise(3, 0.5, 3.0, p), p) *
		0.75*ridged_octavenoise(16*octavenoise(3, 0.275, 2.9, p), 0.3*octavenoise(2, 0.4, 3.0, p), 2.8*ridged_octavenoise(8, 0.35, 2.7, p), p);

	return (n > 0.0? m_maxHeight*n : 0.0);
}
