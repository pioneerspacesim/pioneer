#ifndef _TERRAINNOISE_H
#define _TERRAINNOISE_H

#include "Terrain.h"
#include "perlin.h"

namespace TerrainNoise {

	#define blend(a,b,v) a*(1.0-v)+b*v
	// octavenoise functions return range [0,1] if roughness = 0.5
	inline double octavenoise(const fracdef_t &def, double roughness, const vector3d &p) {
		double n = 0;
		double octaveAmplitude = roughness;
		double jizm = def.frequency;
		for (int i=0; i<def.octaves; i++) {
			n += octaveAmplitude * noise(jizm*p);
			octaveAmplitude *= roughness;
			jizm *= def.lacunarity;
		}
		return (n+1.0)*0.5;
	}

	inline double river_octavenoise(const fracdef_t &def, double roughness, const vector3d &p) {
		double n = 0;
		double octaveAmplitude = roughness;
		double jizm = def.frequency;
		for (int i=0; i<def.octaves; i++) {
			n += octaveAmplitude * fabs(noise(jizm*p));
			octaveAmplitude *= roughness;
			jizm *= def.lacunarity;
		}
		return fabs(n);
	}

	inline double ridged_octavenoise(const fracdef_t &def, double roughness, const vector3d &p) {
		double n = 0;
		double octaveAmplitude = roughness;
		double jizm = def.frequency;
		for (int i=0; i<def.octaves; i++) {
			n += octaveAmplitude * noise(jizm*p);
			octaveAmplitude *= roughness;
			jizm *= def.lacunarity;
		}
		n = 1.0 - fabs(n);
		n *= n;
		return n;
		//return 1.0 - fabs(n);
	}

	inline double billow_octavenoise(const fracdef_t &def, double roughness, const vector3d &p) {
		double n = 0;
		double octaveAmplitude = roughness;
		double jizm = def.frequency;
		for (int i=0; i<def.octaves; i++) {
			n += octaveAmplitude * noise(jizm*p);
			octaveAmplitude *= roughness;
			jizm *= def.lacunarity;
		}
		return (2.0 * fabs(n) - 1.0)+1.0;
	}

	inline double voronoiscam_octavenoise(const fracdef_t &def, double roughness, const vector3d &p) {
		double n = 0;
		double octaveAmplitude = roughness;
		double jizm = def.frequency;
		for (int i=0; i<def.octaves; i++) {
			n += octaveAmplitude * noise(jizm*p);
			octaveAmplitude *= roughness;
			jizm *= def.lacunarity;
		}
		return sqrt(10.0 * fabs(n));
	}

	inline double dunes_octavenoise(const fracdef_t &def, double roughness, const vector3d &p) {
		double n = 0;
		double octaveAmplitude = roughness;
		double jizm = def.frequency;
		for (int i=0; i<3; i++) {
			n += octaveAmplitude * noise(jizm*p);
			octaveAmplitude *= roughness;
			jizm *= def.lacunarity;
		}
		return 1.0 - fabs(n);
	}

	// XXX merge these with their fracdef versions
	inline double octavenoise(int octaves, double roughness, double lacunarity, const vector3d &p) {
		double n = 0;
		double octaveAmplitude = roughness;
		double jizm = 1.0;
		while (octaves--) {
			n += octaveAmplitude * noise(jizm*p);
			octaveAmplitude *= roughness;
			jizm *= lacunarity;
		}
		return (n+1.0)*0.5;
	}

	inline double river_octavenoise(int octaves, double roughness, double lacunarity, const vector3d &p) {
		double n = 0;
		double octaveAmplitude = roughness;
		double jizm = 1.0;
		while (octaves--) {
			n += octaveAmplitude * fabs(noise(jizm*p));
			octaveAmplitude *= roughness;
			jizm *= lacunarity;
		}
		return n;
	}

	inline double ridged_octavenoise(int octaves, double roughness, double lacunarity, const vector3d &p) {
		double n = 0;
		double octaveAmplitude = roughness;
		double jizm = 1.0;
		while (octaves--) {
			n += octaveAmplitude * noise(jizm*p);
			octaveAmplitude *= roughness;
			jizm *= lacunarity;
		}
		n = 1.0 - fabs(n);
		n *= n;
		return n;
	}

	inline double billow_octavenoise(int octaves, double roughness, double lacunarity, const vector3d &p) {
		double n = 0;
		double octaveAmplitude = roughness;
		double jizm = 1.0;
		while (octaves--) {
			n += octaveAmplitude * noise(jizm*p);
			octaveAmplitude *= roughness;
			jizm *= lacunarity;
		}
		return (2.0 * fabs(n) - 1.0)+1.0;
	}

	inline double voronoiscam_octavenoise(int octaves, double roughness, double lacunarity, const vector3d &p) {
		double n = 0;
		double octaveAmplitude = roughness;
		double jizm = 1.0;
		while (octaves--) {
			n += octaveAmplitude * noise(jizm*p);
			octaveAmplitude *= roughness;
			jizm *= lacunarity;
		}
		return sqrt(10.0 * fabs(n));
	}

	// not really a noise function but no better place for it
	inline vector3d interpolate_color(double n, vector3d start, vector3d end) {
		n = Clamp(n, 0.0, 1.0);
		return start*(1.0-n) + end*n;
	}

};

// common colours for earthlike worlds
// XXX better way to do this?
#define rock   octavenoise(GetFracDef(0), 0.65, p)
#define rock2  octavenoise(GetFracDef(1), 0.6, p)*0.6*ridged_octavenoise(GetFracDef(0), 0.55, p)
#define rock3  0.5*ridged_octavenoise(GetFracDef(0), 0.5, p)*voronoiscam_octavenoise(GetFracDef(0), 0.5, p)*ridged_octavenoise(GetFracDef(1), 0.5, p)
#define rock4  0.5*ridged_octavenoise(GetFracDef(1), 0.5, p)*octavenoise(GetFracDef(1), 0.5, p)*octavenoise(GetFracDef(5), 0.5, p)
#define mud    0.1*voronoiscam_octavenoise(GetFracDef(1), 0.5, p)*octavenoise(GetFracDef(1), 0.5, p) * GetFracDef(5).amplitude
#define sand   ridged_octavenoise(GetFracDef(0), 0.4, p)*dunes_octavenoise(GetFracDef(2), 0.4, p) + 0.1*dunes_octavenoise(GetFracDef(1), 0.5, p)
#define sand2  dunes_octavenoise(GetFracDef(0), 0.6, p)*octavenoise(GetFracDef(4), 0.6, p)
#define sand3  dunes_octavenoise(GetFracDef(2), 0.6, p)*dunes_octavenoise(GetFracDef(6), 0.6, p)
#define grass  billow_octavenoise(GetFracDef(1), 0.8, p)
#define grass2 billow_octavenoise(GetFracDef(3), 0.6, p)*voronoiscam_octavenoise(GetFracDef(4), 0.6, p)*river_octavenoise(GetFracDef(5), 0.6, p)
#define forest octavenoise(GetFracDef(1), 0.65, p)*voronoiscam_octavenoise(GetFracDef(2), 0.65, p)
#define water  dunes_octavenoise(GetFracDef(6), 0.6, p)

#define APPLY_SIMPLE_HEIGHT_REGIONS(h) \
{\
	for (unsigned int i = 0; i < m_positions.size(); i++) {\
		if (m_regionTypes[i].Valid) {\
			const vector3d pos = m_positions[i];\
			RegionType &rt = m_regionTypes[i];\
			double th = rt.height; /*target height*/\
			if (pos.Dot(p) > rt.outer) {\
				const double outer = rt.outer;\
				const double inner = rt.inner;\
				/* maximum variation in height with respect to target height*/\
				const double dynamicRangeHeight = 60.0/m_planetRadius; /*in radii*/\
				const double delta_h = fabs(h-th); \
				const double neg = (h-th>0.0) ? 1.0 : -1.0;\
				/* Make up an expression to compress delta_h: */\
				/* Compress delta_h between 0 and 1*/\
				/*    1.1 use compression of the form c = (delta_h+a)/(a+(delta_h+a)) (eqn. 1)*/\
				/*    1.2 this gives c in the interval [0.5, 1] for delta_h [0, +inf] with c=0.5 at delta_h=0.*/\
				/*  2.0 Use compressed_h = dynamic range*(sign(h-th)*(c-0.5)) (eqn. 2) to get h between th-0.5*dynamic range, th+0.5*dynamic range*/\
				/**/\
				/* Choosing a value for a*/\
				/*    3.1 c [0.5, 0.8] occurs when delta_h [a to 3a] (3x difference) which is roughly the expressible range (above or below that the function changes slowly)*/\
				/*    3.2 Find an expression for the expected variation and divide by around 3*/\
				\
				/* It may become necessary calculate expected variation based on intermediate quantities generated (e.g. distribution fractals)*/\
				/* or to store a per planet estimation of variation when fracdefs are calculated.*/\
				const double variationEstimate = rt.heightVariation;\
				const double a = variationEstimate*(1.0/3.0); /* point 3.2 */\
				\
				double hn = h;\
				const double c = (delta_h+a)/(2.0*a+delta_h);/* point 1.1 */\
				const double compressed_h = dynamicRangeHeight*(neg*(c-0.5))+th;/* point 1.3*/\
				h = blend(h, compressed_h, Clamp((pos.Dot(p)-outer)/(inner-outer), 0.0, 1.0)); break; /* blends from compressed height-terrain height as pos goes inner to outer*/\
			}\
		}\
	}\
}

#endif
