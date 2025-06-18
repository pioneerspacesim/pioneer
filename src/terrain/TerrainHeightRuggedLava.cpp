// Copyright © 2008-2025 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#include "Terrain.h"
#include "TerrainFeature.h"
#include "TerrainNoise.h"

using namespace TerrainNoise;
using namespace TerrainFeature;

template <>
const char *TerrainHeightFractal<TerrainHeightRuggedLava>::GetHeightFractalName() const { return "RuggedLava"; }

template <>
TerrainHeightFractal<TerrainHeightRuggedLava>::TerrainHeightFractal(const SystemBody *body) :
	Terrain(body)
{
	SetFracDef(0, m_maxHeightInMeters, m_rand.Double(1e6, 1e7));
	double height = m_maxHeightInMeters * 1.0;
	SetFracDef(1, m_maxHeightInMeters, m_rand.Double(50.0, 100.0) * m_maxHeightInMeters);
	SetFracDef(2, height, m_rand.Double(4.0, 20.0) * height);
	SetFracDef(3, height, m_rand.Double(12.0, 200.0) * height);

	height = m_maxHeightInMeters * 0.3;
	SetFracDef(4, m_maxHeightInMeters, m_rand.Double(100.0, 200.0) * m_maxHeightInMeters);
	SetFracDef(5, height, m_rand.Double(2.5, 3.5) * height);

	// volcanoes
	SetFracDef(6, height, 6e6, 100000.0);
	SetFracDef(7, height, 3e6, 1000.0);

	// canyon
	SetFracDef(8, m_maxHeightInMeters * 0.4, 4e6, 100.0);
	// bumps/rocks
	SetFracDef(9, height * 0.001, m_rand.Double(10, 100), 2.0);
}

template <>
void TerrainHeightFractal<TerrainHeightRuggedLava>::GetHeights(const vector3d *vP, double *heightsOut, const size_t count) const
{
	for (size_t i = 0; i < count; i++) {
		const vector3d &p = vP[i];
		double continents = octavenoise(m_fracdef[0], Clamp(0.725 - (m_sealevel / 2), 0.1, 0.725), p) - m_sealevel;
		if (continents < 0.0)
			heightsOut[i] = 0.0;

		double mountain_distrib = octavenoise(m_fracdef[1], 0.55, p);
		double mountains = octavenoise(m_fracdef[2], 0.5, p) * ridged_octavenoise(m_fracdef[2], 0.575, p);
		double mountains2 = octavenoise(m_fracdef[3], 0.5, p);
		double hill_distrib = octavenoise(m_fracdef[4], 0.5, p);
		double hills = hill_distrib * m_fracdef[5].amplitude * octavenoise(m_fracdef[5], 0.5, p);
		double rocks = octavenoise(m_fracdef[9], 0.5, p);

		double n = continents - (m_fracdef[0].amplitude * m_sealevel);
		//double n = (megavolcano_function(p) + volcano_function(p) + smlvolcano_function(p));
		n += mountains * mountains2 * 5.0 * megavolcano_function(m_fracdef[6], p);
		n += 2.5 * megavolcano_function(m_fracdef[6], p);
		n += mountains * mountains2 * 5.0 * volcano_function(m_fracdef[6], p) * volcano_function(m_fracdef[6], p);
		n += 2.5 * volcano_function(m_fracdef[6], p);

		n += mountains * mountains2 * 7.5 * megavolcano_function(m_fracdef[7], p);
		n += 2.5 * megavolcano_function(m_fracdef[7], p);
		n += mountains * mountains2 * 7.5 * volcano_function(m_fracdef[7], p) * volcano_function(m_fracdef[7], p);
		n += 2.5 * volcano_function(m_fracdef[7], p);

		//n += 1.4*(continents - targ.continents.amplitude*targ.sealevel + (volcano_function(p)*1)) ;
		//smooth canyon transitions and limit height of canyon placement
		if (n < .01)
			n += n * 100.0f * canyon3_ridged_function(m_fracdef[8], p);
		else
			n += canyon3_ridged_function(m_fracdef[8], p);

		if (n < .01)
			n += n * 100.0f * canyon2_ridged_function(m_fracdef[8], p);
		else
			n += canyon2_ridged_function(m_fracdef[8], p);
		n *= 0.5;

		n += continents * hills * hill_distrib * mountain_distrib;

		mountains = octavenoise(m_fracdef[1], 0.5, p) *
			m_fracdef[2].amplitude * mountains * mountains * mountains;
		mountains2 = octavenoise(m_fracdef[4], 0.5, p) *
			m_fracdef[3].amplitude * mountains2 * mountains2 * mountains2;
		/*mountains = fractal(2, targ.mountainDistrib, (m_seed>>2)&3, p) *
			targ.mountains.amplitude * mountains*mountains*mountains;
		mountains2 = fractal(24, targ.mountainDistrib, (m_seed>>2)&3, p) *
			targ.mountains.amplitude * mountains*mountains*mountains;*/

		n += continents * mountains * hill_distrib;
		if (n < 0.01)
			n += continents * mountains2 * n * 40.0f;
		else
			n += continents * mountains2 * .4f;
		n *= 0.2;
		n += mountains * mountains2 * mountains2 * hills * hills * hill_distrib * mountain_distrib * 20.0;

		rocks = continents * mountain_distrib * m_fracdef[9].amplitude * rocks * rocks * rocks * 2.0;
		n += rocks;

		n = (n < 0.0 ? 0.0 : m_maxHeight * n);
		heightsOut[i] = n;
	}
}
