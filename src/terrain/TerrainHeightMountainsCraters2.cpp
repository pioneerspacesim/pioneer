// Copyright © 2008-2026 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#include "Terrain.h"
#include "TerrainFeature.h"
#include "TerrainNoise.h"

using namespace TerrainNoise;
using namespace TerrainFeature;

namespace {
	constexpr double _featureWidthCentre[] = {
		1000000.0,
		 900000.0,
		 800000.0,
		1100000.0,
		1200000.0
	};
	constexpr double _maxHeightMultiplier[] = {
		0.05,
		0.04,
		0.05,
		0.04,
		0.07
	};
}

template <>
const char *TerrainHeightFractal<TerrainHeightMountainsCraters2>::GetHeightFractalName() const { return "MountainsCraters2"; }

template <>
TerrainHeightFractal<TerrainHeightMountainsCraters2>::TerrainHeightFractal(const SystemBody *body) :
	Terrain(body)
{
	const double invRadiusMul = Clamp<double>(1.0 - m_minBody.m_radiusEarthRatio, m_minBody.m_radiusEarthRatio, 1.0);
	SetFracDef(0, m_maxHeightInMeters, m_rand.Double(1e6, 1e7) * invRadiusMul);
	double height = m_maxHeightInMeters * 0.5;
	SetFracDef(1, height, m_rand.Double(50.0, 200.0) * height, 10);
	SetFracDef(2, m_maxHeightInMeters, m_rand.Double(500.0, 5000.0) * m_maxHeightInMeters);
	height = m_maxHeightInMeters * 0.4;
	SetFracDef(3, height, m_rand.Double(2.5, 3.5) * height);
	SetFracDef(4, m_maxHeightInMeters, m_rand.Double(100.0, 200.0) * m_maxHeightInMeters);

	for (size_t i = 0; i < 5; i++) {
		const double featureWidthMetres = GetRandWithinRangeAndMul(m_rand, _featureWidthCentre[i], 0.05, invRadiusMul);
		const double smallestOctaveMeters = GetRandWithinRangeAndMul(m_rand, 10000.0, 0.05, invRadiusMul);
		SetFracDef(i+5, m_maxHeightInMeters * _maxHeightMultiplier[i], featureWidthMetres, smallestOctaveMeters);
	}

	// only used from 5 <> 9 so start at 5
	for (size_t i = 5; i < MAX_FRACDEFS; i++) {
		const double angle = m_rand.Double(0.0, DEG2RAD(360.0));
		const vector3d axis(GetVector3Random(m_rand));
		m_quatDefs[i].SetAxisAngle(angle, axis);
	}
}

template <>
void TerrainHeightFractal<TerrainHeightMountainsCraters2>::GetHeights(const vector3d *vP, double *heightsOut, const size_t count) const
{
	for (size_t i = 0; i < count; i++) {
		const vector3d &p = vP[i];
		double continents = octavenoise(m_fracdef[0], 0.5, p) - m_sealevel;
		if (continents < 0.0) {
			heightsOut[i] = 0.0;
			continue;
		}
		double n = 0.3 * continents;

		double m = octavenoise(m_fracdef[1], 0.5, p);
		double distrib = 0.5 * ridged_octavenoise(m_fracdef[1], 0.5 * octavenoise(m_fracdef[2], 0.5, p), p);
		distrib += 0.7 * billow_octavenoise(m_fracdef[2], 0.5 * ridged_octavenoise(m_fracdef[1], 0.5, p), p) +
			0.1 * octavenoise(m_fracdef[3], 0.5 * ridged_octavenoise(m_fracdef[2], 0.5, p), p);

		if (distrib > 0.5)
			m += 2.0 * (distrib - 0.5) * m_fracdef[3].amplitude * octavenoise(m_fracdef[4], 0.5 * distrib, p);

		// cliffs at shore
		if (continents < 0.001)
			n += m * continents * 1000.0f;
		else
			n += m;

		for (size_t i = 5; i < 10; i++) {
			n += crater_function(m_fracdef[i], m_quatDefs[i] * p);
		}
		n *= m_maxHeight;
		heightsOut[i] = (n > 0.0 ? n : 0.0);
	}
}
