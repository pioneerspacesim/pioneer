#include "Terrain.h"
#include "perlin.h"

namespace TerrainNoise {

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
