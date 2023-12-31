// Copyright © 2008-2024 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#include "Terrain.h"
#include "TerrainNoise.h"

using namespace TerrainNoise;

template <>
const char *TerrainHeightFractal<TerrainHeightMapped>::GetHeightFractalName() const { return "Mapped"; }

template <>
TerrainHeightFractal<TerrainHeightMapped>::TerrainHeightFractal(const SystemBody *body) :
	Terrain(body)
{
	//textures
	SetFracDef(0, m_maxHeightInMeters, 10, 10);
	SetFracDef(1, m_maxHeightInMeters, 25, 10);
	//small fractal/high detail
	SetFracDef(2, m_maxHeightInMeters * 0.0000005, 50, 20); //[2]
	//continental/large type fractal
	SetFracDef(3, m_maxHeightInMeters * 0.00005, 1e6, 800); //[0]
	SetFracDef(4, m_maxHeightInMeters * 0.00005, 1e5, 400); //[4]
	//medium fractal
	SetFracDef(5, m_maxHeightInMeters * 0.000005, 2e4, 200); //[5]
	SetFracDef(6, m_maxHeightInMeters * 0.0000005, 5e3, 100); //[3]
}

template <>
double TerrainHeightFractal<TerrainHeightMapped>::GetHeight(const vector3d &p) const
{
	// This is all used for Earth and Earth alone

	double v = BiCubicInterpolation(p);

	v = (v < 0 ? 0 : v);
	double h = v;

	//Here's where we add some noise over the heightmap so it doesnt look so boring, we scale by height so values are greater high up
	//large mountainous shapes
	double mountains = h * h * 0.001 * octavenoise(GetFracDef(3), 0.5 * octavenoise(GetFracDef(5), 0.45, p), p) * ridged_octavenoise(GetFracDef(4), 0.475 * octavenoise(GetFracDef(6), 0.4, p), p);
	v += mountains;
	//smaller ridged mountains
	if (v < 50.0) {
		v += v * v * 0.04 * ridged_octavenoise(GetFracDef(5), 0.5, p);
	} else if (v < 100.0) {
		v += 100.0 * ridged_octavenoise(GetFracDef(5), 0.5, p);
	} else {
		v += (100.0 / v) * (100.0 / v) * (100.0 / v) * (100.0 / v) * (100.0 / v) *
			100.0 * ridged_octavenoise(GetFracDef(5), 0.5, p);
	}
	//high altitude detail/mountains
	//v += Clamp(h, 0.0, 0.5)*octavenoise(GetFracDef(2-m_fracnum), 0.5, p);

	//low altitude detail/dunes
	//v += h*0.000003*ridged_octavenoise(GetFracDef(2-m_fracnum), Clamp(1.0-h*0.002, 0.0, 0.5), p);
	if (v < 10.0) {
		v += 2.0 * v * dunes_octavenoise(GetFracDef(6), 0.5, p) * octavenoise(GetFracDef(6), 0.5, p);
	} else if (v < 50.0) {
		v += 20.0 * dunes_octavenoise(GetFracDef(6), 0.5, p) * octavenoise(GetFracDef(6), 0.5, p);
	} else {
		v += (50.0 / v) * (50.0 / v) * (50.0 / v) * (50.0 / v) * (50.0 / v) * 20.0 * dunes_octavenoise(GetFracDef(6), 0.5, p) * octavenoise(GetFracDef(6), 0.5, p);
	}
	if (v < 40.0) {
		//v = v;
	} else if (v < 60.0) {
		v += (v - 40.0) * billow_octavenoise(GetFracDef(5), 0.5, p);
		//Output("V/height: %f\n", Clamp(v-20.0, 0.0, 1.0));
	} else {
		v += (30.0 / v) * (30.0 / v) * (30.0 / v) * 20.0 * billow_octavenoise(GetFracDef(5), 0.5, p);
	}

	//ridges and bumps
	//v += h*0.1*ridged_octavenoise(GetFracDef(6-m_fracnum), Clamp(h*0.0002, 0.3, 0.5), p)
	//	* Clamp(h*0.0002, 0.1, 0.5);
	v += h * 0.2 * voronoiscam_octavenoise(GetFracDef(5), Clamp(1.0 - (h * 0.0002), 0.0, 0.6), p) * Clamp(1.0 - (h * 0.0006), 0.0, 1.0);
	//polar ice caps with cracks
	if ((m_icyness * 0.5) + (fabs(p.y * p.y * p.y * 0.38)) > 0.6) {
		h = Clamp(1.0 - (v * 10.0), 0.0, 1.0) * voronoiscam_octavenoise(GetFracDef(5), 0.5, p);
		h *= h * h * 2.0;
		h -= 3.0;
		v += h;
	}

	return v < 0 ? 0 : (v * m_invPlanetRadius);
}
