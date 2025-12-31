// Copyright © 2008-2026 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#include "Terrain.h"
#include "TerrainFeature.h"
#include "TerrainNoise.h"

using namespace TerrainNoise;
using namespace TerrainFeature;

template <>
const char *TerrainHeightFractal<TerrainHeightWaterSolid>::GetHeightFractalName() const { return "WaterSolid"; }

template <>
TerrainHeightFractal<TerrainHeightWaterSolid>::TerrainHeightFractal(const SystemBody *body) :
	Terrain(body)
{
	SetFracDef(0, m_maxHeightInMeters, m_rand.Double(5e6, 1e8));
	double height = m_maxHeightInMeters * 0.3;
	SetFracDef(1, height, m_rand.Double(4.0, 20.0) * height);
	SetFracDef(2, m_maxHeightInMeters, m_rand.Double(200.0, 1000.0) * m_maxHeightInMeters);

	// mountains with some canyons
	SetFracDef(3, m_maxHeightInMeters * 0.4, 4e6);
	SetFracDef(4, m_maxHeightInMeters * 0.4, 5e6);
	//crater
	SetFracDef(5, m_maxHeightInMeters * 0.4, 1.5e7, 50000.0);
}

template <>
void TerrainHeightFractal<TerrainHeightWaterSolid>::GetHeights(const vector3d *vP, double *heightsOut, const size_t count) const
{
	for (size_t i = 0; i < count; i++) {
		const vector3d &p = vP[i];
		double continents = 0.7 * river_octavenoise(m_fracdef[2], 0.5, p) - m_sealevel;
		continents = m_fracdef[0].amplitude * ridged_octavenoise(m_fracdef[0], Clamp(continents, 0.0, 0.6), p);
		double mountains = ridged_octavenoise(m_fracdef[2], 0.5, p);
		double hills = octavenoise(m_fracdef[2], 0.5, p) *
			m_fracdef[1].amplitude * river_octavenoise(m_fracdef[1], 0.5, p);
		double n = continents - (m_fracdef[0].amplitude * m_sealevel);
		// craters
		n += crater_function(m_fracdef[5], p);
		if (n > 0.0) {
			// smooth in hills at shore edges
			if (n < 0.05) {
				n += hills * n * 4.0;
				n += n * 20.0 * (billow_octavenoise(m_fracdef[3], 0.5 * ridged_octavenoise(m_fracdef[2], 0.5, p), p) + river_octavenoise(m_fracdef[4], 0.5 * ridged_octavenoise(m_fracdef[3], 0.5, p), p) + billow_octavenoise(m_fracdef[3], 0.6 * ridged_octavenoise(m_fracdef[4], 0.55, p), p));
			} else {
				n += hills * .2f;
				n += billow_octavenoise(m_fracdef[3], 0.5 * ridged_octavenoise(m_fracdef[2], 0.5, p), p) +
					river_octavenoise(m_fracdef[4], 0.5 * ridged_octavenoise(m_fracdef[3], 0.5, p), p) +
					billow_octavenoise(m_fracdef[3], 0.6 * ridged_octavenoise(m_fracdef[4], 0.55, p), p);
			}
			// adds mountains hills craters
			mountains = octavenoise(m_fracdef[3], 0.5, p) *
				m_fracdef[2].amplitude * mountains * mountains * mountains;
			if (n < 0.4)
				n += 2.0 * n * mountains;
			else
				n += mountains * .8f;
		}
		n *= m_maxHeight;
		n = (n < 0.0 ? -n : n);
		n = (n > 1.0 ? 2.0 - n : n);
		heightsOut[i] = n;
	}
}
