// Copyright © 2008-2026 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#include "Terrain.h"
#include "TerrainFeature.h"
#include "TerrainNoise.h"

using namespace TerrainNoise;
using namespace TerrainFeature;

namespace {
	constexpr double _featureWidthCentre[] = {
		1100000.0,
		 980000.0,
		1000000.0,
		 990000.0,
		1200000.0,
		 900000.0
	};
	constexpr double _smallestOctaveCentre[] = {
		1000.0,
		 800.0,
		 400.0,
		 200.0,
		 100.0,
		 100.0
	};
	constexpr double _maxHeightMultiplier[] = {
		0.07,
		0.05,
		0.05,
		0.04,
		0.05,
		0.04
	};
} //namespace

template <>
const char *TerrainHeightFractal<TerrainHeightHillsCraters2>::GetHeightFractalName() const { return "HillsCraters2"; }

template <>
TerrainHeightFractal<TerrainHeightHillsCraters2>::TerrainHeightFractal(const SystemBody *body) :
	Terrain(body)
{
	const double invRadiusMul = Clamp<double>(1.0 - m_minBody.m_radiusEarthRatio, 0.01, 1.0);
	SetFracDef(0, m_maxHeightInMeters, m_rand.Double(1e6, 1e7) * invRadiusMul);
	const double height = m_maxHeightInMeters * 0.6;
	SetFracDef(1, height, m_rand.Double(4.0, 20.0) * height);
	SetFracDef(2, m_maxHeightInMeters, m_rand.Double(50.0, 100.0) * m_maxHeightInMeters);

	for (size_t i = 0; i < 6; i++) {
		const double featureWidthMetres = GetRandWithinRangeAndMul(m_rand, _featureWidthCentre[i], 0.05, invRadiusMul);
		const double smallestOctaveMeters = GetRandWithinRangeAndMul(m_rand, _smallestOctaveCentre[i], 0.05, invRadiusMul);
		SetFracDef(i + 3, m_maxHeightInMeters * _maxHeightMultiplier[i], featureWidthMetres, smallestOctaveMeters);
	}

	// only used from 3 <> 8 so start at 3
	for (size_t i = 3; i < 9; i++) {
		const double angle = m_rand.Double(0.0, DEG2RAD(360.0));
		const vector3d axis(GetVector3Random(m_rand));
		m_quatDefs[i].SetAxisAngle(angle, axis);
	}
}

template <>
void TerrainHeightFractal<TerrainHeightHillsCraters2>::GetHeights(const vector3d *vP, double *heightsOut, const size_t count) const
{
	for (size_t i = 0; i < count; i++) {
		const vector3d &p = vP[i];
		const double continents = octavenoise(m_fracdef[0], 0.5, p) - m_sealevel;
		if (continents < 0.0){
			heightsOut[i] = 0.0;
			continue;
		}

		// == TERRAIN_HILLS_NORMAL except river_octavenoise
		double n = 0.3 * continents;
		const double distrib = river_octavenoise(m_fracdef[2], 0.5, p);
		const double m = m_fracdef[1].amplitude * river_octavenoise(m_fracdef[1], 0.5 * distrib, p);

		// cliffs at shore
		if (continents < 0.001)
			n += m * continents * 1000.0f;
		else
			n += m;

		for (size_t i = 3; i < 9; i++) {
			n += crater_function(m_fracdef[i], m_quatDefs[i] * p);
		}
		n *= m_maxHeight;
		heightsOut[i] = (n > 0.0 ? n : 0.0);
	}
}
