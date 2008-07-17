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

struct systemloc_t {
	int secX, secY, sysIdx;
};

// all masses in Kg, all lengths in meters

class StarSystem {
public:
	StarSystem(int sector_x, int sector_y, int system_idx);
	~StarSystem();
	bool IsSystem(int sector_x, int sector_y, int system_idx);
	void GetPos(systemloc_t *l) { *l = loc;	}
	void GetPos(int *sec_x, int *sec_y, int *sys_idx) {
		*sec_x = loc.secX; *sec_y = loc.secY; *sys_idx = loc.sysIdx;
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
	
	enum BodyType {
		TYPE_STAR_M,
		TYPE_STAR_K,
		TYPE_STAR_G,
		TYPE_STAR_F,
		TYPE_STAR_A,
		TYPE_STAR_B,
		TYPE_STAR_O,
		TYPE_BROWN_DWARF,
		TYPE_PLANET_SMALL_GAS_GIANT,
		TYPE_PLANET_MEDIUM_GAS_GIANT,
		TYPE_PLANET_LARGE_GAS_GIANT,
		TYPE_PLANET_VERY_LARGE_GAS_GIANT,
		TYPE_PLANET_DWARF,
		TYPE_PLANET_SMALL,
		TYPE_PLANET_WATER,
		TYPE_PLANET_CO2,
		TYPE_PLANET_METHANE,
		TYPE_PLANET_WATER_THICK_ATMOS,
		TYPE_PLANET_CO2_THICK_ATMOS,
		TYPE_PLANET_METHANE_THICK_ATMOS,
		TYPE_PLANET_HIGHLY_VOLCANIC,
		TYPE_PLANET_INDIGENOUS_LIFE,
		TYPE_MAX
		// XXX need larger atmosphereless thing
	};

	enum BodySuperType {
		SUPERTYPE_STAR, SUPERTYPE_ROCKY_PLANET, SUPERTYPE_GAS_GIANT
	};

	struct BodyStats {

	};

	struct SBody {
		~SBody();
		void EliminateBadChildren();
		void PickPlanetType(SBody *, double distToPrimary, MTRand &drand, bool genMoons);
		SBody *parent;
		std::vector<SBody*> children;

		const char *GetAstroDescription();
		const char *GetIcon();

		int temp;
		Orbit orbit;
		int seed; // Planet.cpp can use to generate terrain
		std::string name;
		double radius; 
		double mass;
		double radMin, radMax;
		double averageTemp;
		BodySuperType supertype;
		BodyType type;
	};
	
	SBody *rootBody;
private:
	systemloc_t loc;

	MTRand rand;
};

#endif /* _STARSYSTEM_H */
