// Copyright © 2008-2024 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#include "FracDef.h"
#include "TerrainFeature.h"
#include "TerrainNoise.h"

using namespace TerrainNoise;

namespace TerrainFeature {

	// Creates small canyons.
	double canyon_ridged_function(const fracdef_t &def, const vector3d &p)
	{
		double h;
		double n = 0;
		n = ridged_octavenoise(def.octaves, 0.54, 2.0, def.frequency * p);
		const double outer = 0.71;
		const double inner = 0.715;
		const double inner2 = 0.715;
		const double outer2 = 0.72;
		if (n > outer2) {
			h = 1;
		} else if (n > inner2) {
			h = 0.0 + 1.0 * (n - inner2) * (1.0 / (outer2 - inner2));
		} else if (n > inner) {
			h = 0;
		} else if (n > outer) {
			h = 1.0 - 1.0 * (n - outer) * (1.0 / (inner - outer));
		} else {
			h = 1.0;
		}
		return h * def.amplitude;
	}

	// Larger canyon.
	double canyon2_ridged_function(const fracdef_t &def, const vector3d &p)
	{
		double h;
		double n = 0; //octavenoise(def.octaves, 0.56, 2.0, def.frequency*p);
		n = ridged_octavenoise(def.octaves, 0.56, 2.0, def.frequency * p);
		const double outer = 0.7;
		const double inner = 0.71;
		const double inner2 = 0.72;
		const double outer2 = 0.73;
		if (n > outer2) {
			h = 1;
		} else if (n > inner2) {
			h = 0.0 + 1.0 * (n - inner2) * (1.0 / (outer2 - inner2));
		} else if (n > inner) {
			h = 0;
		} else if (n > outer) {
			h = 1.0 - 1.0 * (n - outer) * (1.0 / (inner - outer));
		} else {
			h = 1.0;
		}
		return h * def.amplitude;
	}

	// Largest and best looking canyon, combine them together for best results.
	double canyon3_ridged_function(const fracdef_t &def, const vector3d &p)
	{
		double h;
		double n = 0; //octavenoise(def.octaves, 0.585, 2.0, def.frequency*p);
		n = ridged_octavenoise(def.octaves, 0.585, 2.0, def.frequency * p);
		const double outer = 0.7;
		const double inner = 0.71;
		const double inner2 = 0.72;
		const double outer2 = 0.73;
		if (n > outer2) {
			h = 1.0;
		} else if (n > inner2) {
			h = 0.0 + 1.0 * (n - inner2) * (1.0 / (outer2 - inner2));
		} else if (n > inner) {
			h = 0.0;
		} else if (n > outer) {
			h = 1.0 - 1.0 * (n - outer) * (1.0 / (inner - outer));
		} else {
			h = 1.0;
		}
		return h * def.amplitude;
	}

	double canyon_normal_function(const fracdef_t &def, const vector3d &p)
	{
		double h;
		double n = 0;
		n = octavenoise(def.octaves, 0.54, 2.0, def.frequency * p);
		const double outer = 0.71;
		const double inner = 0.715;
		const double inner2 = 0.715;
		const double outer2 = 0.72;
		if (n > outer2) {
			h = 1;
		} else if (n > inner2) {
			h = 0.0 + 1.0 * (n - inner2) * (1.0 / (outer2 - inner2));
		} else if (n > inner) {
			h = 0;
		} else if (n > outer) {
			h = 1.0 - 1.0 * (n - outer) * (1.0 / (inner - outer));
		} else {
			h = 1.0;
		}
		return h * def.amplitude;
	}

	double canyon2_normal_function(const fracdef_t &def, const vector3d &p)
	{
		double h;
		double n = 0;
		n = octavenoise(def.octaves, 0.56, 2.0, def.frequency * p);
		const double outer = 0.7;
		const double inner = 0.71;
		const double inner2 = 0.72;
		const double outer2 = 0.73;
		if (n > outer2) {
			h = 1;
		} else if (n > inner2) {
			h = 0.0 + 1.0 * (n - inner2) * (1.0 / (outer2 - inner2));
		} else if (n > inner) {
			h = 0;
		} else if (n > outer) {
			h = 1.0 - 1.0 * (n - outer) * (1.0 / (inner - outer));
		} else {
			h = 1.0;
		}
		return h * def.amplitude;
	}

	double canyon3_normal_function(const fracdef_t &def, const vector3d &p)
	{
		double h;
		double n = 0;
		n = octavenoise(def.octaves, 0.585, 2.0, def.frequency * p);
		const double outer = 0.7;
		const double inner = 0.71;
		const double inner2 = 0.72;
		const double outer2 = 0.73;
		if (n > outer2) {
			h = 1.0;
		} else if (n > inner2) {
			h = 0.0 + 1.0 * (n - inner2) * (1.0 / (outer2 - inner2));
		} else if (n > inner) {
			h = 0.0;
		} else if (n > outer) {
			h = 1.0 - 1.0 * (n - outer) * (1.0 / (inner - outer));
		} else {
			h = 1.0;
		}
		return h * def.amplitude;
	}

	double canyon_voronoi_function(const fracdef_t &def, const vector3d &p)
	{
		double h;
		double n = 0;
		n = octavenoise(def.octaves, 0.54, 2.0, def.frequency * p);
		const double outer = 0.71;
		const double inner = 0.715;
		const double inner2 = 0.715;
		const double outer2 = 0.72;
		if (n > outer2) {
			h = 1;
		} else if (n > inner2) {
			h = 0.0 + 1.0 * (n - inner2) * (1.0 / (outer2 - inner2));
		} else if (n > inner) {
			h = 0;
		} else if (n > outer) {
			h = 1.0 - 1.0 * (n - outer) * (1.0 / (inner - outer));
		} else {
			h = 1.0;
		}
		return h * def.amplitude;
	}

	double canyon2_voronoi_function(const fracdef_t &def, const vector3d &p)
	{
		double h;
		double n = 0;
		n = octavenoise(def.octaves, 0.56, 2.0, def.frequency * p);
		const double outer = 0.7;
		const double inner = 0.71;
		const double inner2 = 0.72;
		const double outer2 = 0.73;
		if (n > outer2) {
			h = 1;
		} else if (n > inner2) {
			h = 0.0 + 1.0 * (n - inner2) * (1.0 / (outer2 - inner2));
		} else if (n > inner) {
			h = 0;
		} else if (n > outer) {
			h = 1.0 - 1.0 * (n - outer) * (1.0 / (inner - outer));
		} else {
			h = 1.0;
		}
		return h * def.amplitude;
	}

	double canyon3_voronoi_function(const fracdef_t &def, const vector3d &p)
	{
		double h;
		double n = 0;
		n = octavenoise(def.octaves, 0.585, 2.0, def.frequency * p);
		const double outer = 0.7;
		const double inner = 0.71;
		const double inner2 = 0.72;
		const double outer2 = 0.73;
		if (n > outer2) {
			h = 1.0;
		} else if (n > inner2) {
			h = 0.0 + 1.0 * (n - inner2) * (1.0 / (outer2 - inner2));
		} else if (n > inner) {
			h = 0.0;
		} else if (n > outer) {
			h = 1.0 - 1.0 * (n - outer) * (1.0 / (inner - outer));
		} else {
			h = 1.0;
		}
		return h * def.amplitude;
	}

	double canyon_billow_function(const fracdef_t &def, const vector3d &p)
	{
		double h;
		double n = 0;
		n = octavenoise(def.octaves, 0.54, 2.0, def.frequency * p);
		const double outer = 0.71;
		const double inner = 0.715;
		const double inner2 = 0.715;
		const double outer2 = 0.72;
		if (n > outer2) {
			h = 1;
		} else if (n > inner2) {
			h = 0.0 + 1.0 * (n - inner2) * (1.0 / (outer2 - inner2));
		} else if (n > inner) {
			h = 0;
		} else if (n > outer) {
			h = 1.0 - 1.0 * (n - outer) * (1.0 / (inner - outer));
		} else {
			h = 1.0;
		}
		return h * def.amplitude;
	}

	double canyon2_billow_function(const fracdef_t &def, const vector3d &p)
	{
		double h;
		double n = 0;
		n = octavenoise(def.octaves, 0.56, 2.0, def.frequency * p);
		const double outer = 0.7;
		const double inner = 0.71;
		const double inner2 = 0.72;
		const double outer2 = 0.73;
		if (n > outer2) {
			h = 1;
		} else if (n > inner2) {
			h = 0.0 + 1.0 * (n - inner2) * (1.0 / (outer2 - inner2));
		} else if (n > inner) {
			h = 0;
		} else if (n > outer) {
			h = 1.0 - 1.0 * (n - outer) * (1.0 / (inner - outer));
		} else {
			h = 1.0;
		}
		return h * def.amplitude;
	}

	double canyon3_billow_function(const fracdef_t &def, const vector3d &p)
	{
		double h;
		double n = 0;
		n = octavenoise(def.octaves, 0.585, 2.0, def.frequency * p);
		const double outer = 0.7;
		const double inner = 0.71;
		const double inner2 = 0.72;
		const double outer2 = 0.73;
		if (n > outer2) {
			h = 1.0;
		} else if (n > inner2) {
			h = 0.0 + 1.0 * (n - inner2) * (1.0 / (outer2 - inner2));
		} else if (n > inner) {
			h = 0.0;
		} else if (n > outer) {
			h = 1.0 - 1.0 * (n - outer) * (1.0 / (inner - outer));
		} else {
			h = 1.0;
		}
		return h * def.amplitude;
	}

	/*double rock_function(const fracdef_t &def, const vector3d &p)
{
	double h;
	double n = octavenoise(def.octaves, 0.788, 2.0, def.frequency*p);
	const double outer = 0.1;
	const double inner = 0.3;
	const double inner2 = 0.7;
	const double outer2 = 0.9;
	if (n > outer2) {
		h = 0.0;
	} else if (n > inner2) {
		h = 1.0 - ((n-inner2)*(1.0/(outer2-inner2)));
	} else if (n > inner) {
		h = 1.0;
	} else if (n > outer) {
		h = 0.0+1.0*(n-outer)*(1.0/(inner-outer));
	} else {
		h = 0.0;
	}
	return h * def.amplitude;
}*/

	void crater_function_1pass(const vector3d &p, double &out, const double height)
	{
		double n = fabs(noise(p));
		const double ejecta_outer = 0.6;
		const double outer = 0.9;
		const double inner = 0.94;
		const double midrim = 0.93;
		if (n > inner) {
			//out = 0;
		} else if (n > midrim) {
			double hrim = inner - midrim;
			double descent = (hrim - (n - midrim)) / hrim;
			out += height * descent * descent;
		} else if (n > outer) {
			double hrim = midrim - outer;
			double ascent = (n - outer) / hrim;
			out += height * ascent * ascent;
		} else if (n > ejecta_outer) {
			// blow down walls of other craters too near this one,
			// so we don't have sharp transition
			//out *= (outer-n)/-(ejecta_outer-outer);
		}
	}

	// makes large and small craters across the entire planet.
	double crater_function(const fracdef_t &def, const vector3d &p)
	{
		double crater = 0.0;
		double sz = def.frequency;
		double max_h = def.amplitude;
		for (int i = 0; i < def.octaves; i++) {
			crater_function_1pass(sz * p, crater, max_h);
			sz *= 2.0;
			max_h *= 0.5;
		}
		return crater;
	}

	void impact_crater_function_1pass(const vector3d &p, double &out, const double height)
	{
		double n = fabs(noise(p));
		const double ejecta_outer = 0.6;
		const double outer = 0.9;
		const double midrim = 0.93;
		double hrim;
		double descent;
		if (n > midrim) {
			out -= height;
		} else if (n > outer) {
			hrim = midrim - outer;
			descent = (n - outer) / hrim;
			out -= height * descent * descent;
		} else if (n > ejecta_outer) {
			// blow down walls of other craters too near this one,
			// so we don't have sharp transition
			//out *= (outer-n)/-(ejecta_outer-outer);
		}
	}

	// makes large and small craters across the entire planet.
	double impact_crater_function(const fracdef_t &def, const vector3d &p)
	{
		double crater = 0.0;
		double sz = def.frequency;
		double max_h = def.amplitude;
		for (int i = 0; i < def.octaves; i++) {
			impact_crater_function_1pass(sz * p, crater, max_h);
			sz *= 2.0;
			max_h *= 0.5;
		}
		return crater;
	}

	void volcano_function_1pass(const vector3d &p, double &out, const double height)
	{
		double n = fabs(noise(p));
		const double ejecta_outer = 0.6;
		const double outer = 0.9;
		const double inner = 0.975;
		const double midrim = 0.971;
		if (n > inner) {
			//out = 0;
		} else if (n > midrim) {
			double hrim = inner - midrim;
			double descent = (hrim - (n - midrim)) / hrim;
			out += height * descent;
		} else if (n > outer) {
			double hrim = midrim - outer;
			double ascent = (n - outer) / hrim;
			out += height * ascent * ascent;
		} else if (n > ejecta_outer) {
			// blow down walls of other craters too near this one,
			// so we don't have sharp transition
			out *= (outer - n) / -(ejecta_outer - outer);
		}
	}

	double volcano_function(const fracdef_t &def, const vector3d &p)
	{
		double crater = 0.0;
		double sz = def.frequency;
		double max_h = def.amplitude;
		for (int i = 0; i < def.octaves; i++) {
			volcano_function_1pass(sz * p, crater, max_h);
			sz *= 1.0; //frequency?
			max_h *= 0.4; // height??
		}
		return 3.0 * crater;
	}

	void megavolcano_function_1pass(const vector3d &p, double &out, const double height)
	{
		double n = fabs(noise(p));
		const double ejecta_outer = 0.6;
		const double outer = 0.76; //Radius
		const double inner = 0.98;
		const double midrim = 0.964;
		if (n > inner) {
			//out = 0;
		} else if (n > midrim) {
			double hrim = inner - midrim;
			double descent = (hrim - (n - midrim)) / hrim;
			out += height * descent;
		} else if (n > outer) {
			double hrim = midrim - outer;
			double ascent = (n - outer) / hrim;
			out += height * ascent * ascent;
		} else if (n > ejecta_outer) {
			// blow down walls of other craters too near this one,
			// so we don't have sharp transition
			out *= (outer - n) / -(ejecta_outer - outer);
		}
	}

	double megavolcano_function(const fracdef_t &def, const vector3d &p)
	{
		double crater = 0.0;
		double sz = def.frequency;
		double max_h = def.amplitude;
		for (int i = 0; i < def.octaves; i++) {
			megavolcano_function_1pass(sz * p, crater, max_h);
			sz *= 1.0; //frequency?
			max_h *= 0.15; // height??
		}
		return 4.0 * crater;
	}

	double river_function(const fracdef_t &def, const vector3d &p, int style)
	{
		assert(style >= 0 && style < 2);
		double h;
		double n = octavenoise(def.octaves, 0.585, 2.0, def.frequency * p * 0.5);
		const double outer[] = { 0.67, 0.01 };
		const double inner[] = { 0.715, 0.49 };
		const double inner2[] = { 0.715, 0.51 };
		const double outer2[] = { 0.76, 0.99 };
		if (n > outer2[style]) {
			h = 1;
		} else if (n > inner2[style]) {
			h = 0.0 + 1.0 * (n - inner2[style]) * (1.0 / (outer2[style] - inner2[style]));
		} else if (n > inner[style]) {
			h = 0;
		} else if (n > outer[style]) {
			h = 1.0 - 1.0 * (n - outer[style]) * (1.0 / (inner[style] - outer[style]));
		} else {
			h = 1.0;
		}
		return h * def.amplitude;
	}

// Original canyon function, But really it generates cliffs.
#if 0
double cliff_function(const fracdef_t &def, const vector3d &p)
{
	double h;
	double n = octavenoise(def.octaves, 0.54, 2.0, def.frequency*p);
	const double outer = 0.7;
	const double inner = 0.71;
	if (n > inner) {
		h = 0;
	} else if (n > outer) {
		h = 1.0-1.0*(n-outer)*(1.0/(inner-outer));
	} else {
		h = 1.0;
	}
	return h;
}
#endif

} // namespace TerrainFeature
