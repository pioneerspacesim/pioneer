// Copyright © 2008-2026 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#include "Terrain.h"
#include "TerrainFeature.h"
#include "TerrainNoise.h"

using namespace TerrainNoise;
using namespace TerrainFeature;

template <>
const char *TerrainHeightFractal<TerrainHeightMountainsVolcano>::GetHeightFractalName() const { return "MountainsVolcano"; }

template <>
TerrainHeightFractal<TerrainHeightMountainsVolcano>::TerrainHeightFractal(const SystemBody *body) :
	Terrain(body)
{
	SetFracDef(0, m_maxHeightInMeters, m_rand.Double(1e6, 1e7));
	double height = m_maxHeightInMeters * 0.8;
	SetFracDef(1, m_maxHeightInMeters, m_rand.Double(50.0, 100.0) * m_maxHeightInMeters);
	SetFracDef(2, height, m_rand.Double(4.0, 20.0) * height);
	SetFracDef(3, height, m_rand.Double(12.0, 200.0) * height);

	height = m_maxHeightInMeters * 0.7;
	SetFracDef(4, m_maxHeightInMeters, m_rand.Double(100.0, 200.0) * m_maxHeightInMeters);
	SetFracDef(5, height, m_rand.Double(2.5, 3.5) * height);
	SetFracDef(6, height, m_rand.Double(2.5, 3.5) * height);
	// volcano
	SetFracDef(7, 20000.0, 5000000.0, 1000.0);

	// canyons
	SetFracDef(8, m_maxHeightInMeters * 0.5, 2e6, 100.0);
	//SetFracDef(9, m_maxHeightInMeters*0.1, 1.5e6, 100.0*m_fracmult);
	//SetFracDef(10, m_maxHeightInMeters*0.1, 2e6, 100.0*m_fracmult);
}

template <>
void TerrainHeightFractal<TerrainHeightMountainsVolcano>::GetHeights(const vector3d *vP, double *heightsOut, const size_t count) const
{
	for (size_t i = 0; i < count; i++) {
		const vector3d &p = vP[i];
		double continents = octavenoise(m_fracdef[0], 0.5, p) - m_sealevel;
		if (continents < 0.0)
			heightsOut[i] = 0.0;

		// unused variable \\ double mountain_distrib = octavenoise(m_fracdef[1], 0.5, p);
		double mountains = octavenoise(m_fracdef[2], 0.5, p);
		double mountains2 = octavenoise(m_fracdef[3], 0.5, p);
		double hill_distrib = octavenoise(m_fracdef[4], 0.5, p);
		double hills = hill_distrib * m_fracdef[5].amplitude * octavenoise(m_fracdef[5], 0.5, p);
		double hills2 = hill_distrib * m_fracdef[6].amplitude * octavenoise(m_fracdef[6], 0.5, p);

		double n = continents - (m_fracdef[0].amplitude * m_sealevel);

		if (n < 0.01)
			n += megavolcano_function(m_fracdef[7], p) * n * 3000.0f;
		else
			n += megavolcano_function(m_fracdef[7], p) * 30.0f;

		n = (n > 0.0 ? n : 0.0);

		if ((m_seed >> 2) % 3 > 2) {
			if (n < .2f)
				n += canyon3_ridged_function(m_fracdef[8], p) * n * 2;
			else if (n < .4f)
				n += canyon3_ridged_function(m_fracdef[8], p) * .4;
			else
				n += canyon3_ridged_function(m_fracdef[8], p) * (.4 / n) * .4;
		} else if ((m_seed >> 2) % 3 > 1) {
			if (n < .2f)
				n += canyon3_billow_function(m_fracdef[8], p) * n * 2;
			else if (n < .4f)
				n += canyon3_billow_function(m_fracdef[8], p) * .4;
			else
				n += canyon3_billow_function(m_fracdef[8], p) * (.4 / n) * .4;
		} else {
			if (n < .2f)
				n += canyon3_voronoi_function(m_fracdef[8], p) * n * 2;
			else if (n < .4f)
				n += canyon3_voronoi_function(m_fracdef[8], p) * .4;
			else
				n += canyon3_voronoi_function(m_fracdef[8], p) * (.4 / n) * .4;
		}

		n += -0.05f;
		n = (n > 0.0 ? n : 0.0);

		n = n * .01f;

		if (n > 0.0) {
			// smooth in hills at shore edges
			if (n < 0.01)
				n += hills * n * 100.0f;
			else
				n += hills;
			if (n < 0.02)
				n += hills2 * n * 50.0f;
			else
				n += hills2 * (0.02f / n);

			mountains = octavenoise(m_fracdef[1], 0.5, p) *
				m_fracdef[2].amplitude * mountains * mountains * mountains;
			mountains2 = octavenoise(m_fracdef[4], 0.5, p) *
				m_fracdef[3].amplitude * mountains2 * mountains2 * mountains2;
			if (n > 2.5) n += mountains2 * (n - 2.5) * 0.6f;
			if (n < 0.01)
				n += mountains * n * 60.0f;
			else
				n += mountains * 0.6f;
		}

		n *= m_maxHeight;
		heightsOut[i] = (n > 0.0 ? n : 0.0);
	}
}
