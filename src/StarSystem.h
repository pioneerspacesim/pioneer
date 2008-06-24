#ifndef _STARSYSTEM_H
#define _STARSYSTEM_H

#include "libs.h"
#include <vector>
#include <string>

#define EARTH_RADIUS	6378135.0
#define EARTH_MASS	5.9742e24
#define JUPITER_MASS	(317.8*EARTH_MASS)
// brown dwarfs above 13 jupiter masses fuse deuterium
#define MIN_BROWN_DWARF	(13.0*JUPITER_MASS)
#define SOL_RADIUS	6.955e8
#define SOL_MASS	1.98892e30
#define AU		149598000000.0
#define G		6.67428e-11


// all masses in Kg, all lengths in meters

class StarSystem {
public:
	StarSystem(int sector_x, int sector_y, int system_idx);
	~StarSystem();
	bool IsSystem(int sector_x, int sector_y, int system_idx);
	void GetPos(int *sec_x, int *sec_y, int *sys_idx) {
		*sec_x = m_sectorX; *sec_y = m_sectorY; *sys_idx = m_systemIdx;
	}

	static float starColors[7][3];

	struct Orbit {
		void KeplerPosAtTime(double t, double *dist, double *ang);
		vector3d CartesianPosAtTime(double t);
		double eccentricity;
		double semiMajorAxis;
		double period; // seconds
		matrix4x4d rotMatrix;
	};

	struct SBody {
		~SBody();
		void EliminateBadChildren();
		void PickPlanetType(SBody *, double distToPrimary, MTRand &drand, bool genMoons);
		SBody *parent;
		std::vector<SBody*> children;
		Orbit orbit;

		const char *GetAstroDescription();
		const char *GetIcon();

		int temp;
		std::string name;
		double radius; 
		double mass;
		double radMin, radMax;
		double averageTemp;
		enum {
			TYPE_STAR, TYPE_ROCKY_PLANET, TYPE_GAS_GIANT
		} type;
		enum SubType {
			SUBTYPE_STAR_M,
			SUBTYPE_STAR_K,
			SUBTYPE_STAR_G,
			SUBTYPE_STAR_F,
			SUBTYPE_STAR_A,
			SUBTYPE_STAR_B,
			SUBTYPE_STAR_O,
			SUBTYPE_BROWN_DWARF,
			SUBTYPE_PLANET_SMALL_GAS_GIANT,
			SUBTYPE_PLANET_MEDIUM_GAS_GIANT,
			SUBTYPE_PLANET_LARGE_GAS_GIANT,
			SUBTYPE_PLANET_VERY_LARGE_GAS_GIANT,
			SUBTYPE_PLANET_DWARF,
			SUBTYPE_PLANET_SMALL,
			SUBTYPE_PLANET_WATER,
			SUBTYPE_PLANET_CO2,
			SUBTYPE_PLANET_METHANE,
			SUBTYPE_PLANET_WATER_THICK_ATMOS,
			SUBTYPE_PLANET_CO2_THICK_ATMOS,
			SUBTYPE_PLANET_METHANE_THICK_ATMOS,
			SUBTYPE_PLANET_HIGHLY_VOLCANIC,
			SUBTYPE_PLANET_INDIGENOUS_LIFE,
			SUBTYPE_MAX
			// XXX need larger atmosphereless thing
		} subtype;
	};
	
	SBody *rootBody;
private:
	int m_sectorX, m_sectorY, m_systemIdx;

	MTRand rand;
};

#endif /* _STARSYSTEM_H */
