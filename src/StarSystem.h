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

struct CustomSBody;

// doubles: all masses in Kg, all lengths in meters
// fixed: any mad scheme

class StarSystem {
public:
	StarSystem() { rootBody = 0; }
	StarSystem(int sector_x, int sector_y, int system_idx);
	~StarSystem();
	bool IsSystem(int sector_x, int sector_y, int system_idx);
	void GetPos(int *sec_x, int *sec_y, int *sys_idx) {
		*sec_x = m_secx; *sec_y = m_secy; *sys_idx = m_sysIdx;
	}

	static float starColors[][3];

	struct Orbit {
		void KeplerPosAtTime(double t, double *dist, double *ang);
		vector3d CartesianPosAtTime(double t);
		double eccentricity;
		double semiMajorAxis;
		double period; // seconds
		matrix4x4d rotMatrix;
	};
	
	enum BodyType {
		TYPE_GRAVPOINT,
		TYPE_STAR_M,
		TYPE_STAR_K,
		TYPE_STAR_G,
		TYPE_STAR_F,
		TYPE_STAR_A,
		TYPE_STAR_B,
		TYPE_STAR_O,
		TYPE_WHITE_DWARF,
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
		TYPE_MAX,
		TYPE_STAR_MIN = TYPE_STAR_M,
		TYPE_STAR_MAX = TYPE_WHITE_DWARF
		// XXX need larger atmosphereless thing
	};

	enum BodySuperType {
		SUPERTYPE_NONE, SUPERTYPE_STAR, SUPERTYPE_ROCKY_PLANET, SUPERTYPE_GAS_GIANT
	};

	struct BodyStats {

	};

	class SBody {
	public:
		friend class StarSystem;

		~SBody();
		void EliminateBadChildren();
		void PickPlanetType(SBody *, fixed distToPrimary, MTRand &drand, bool genMoons);
		SBody *parent;
		std::vector<SBody*> children;

		const char *GetAstroDescription();
		const char *GetIcon();
		BodySuperType GetSuperType() const;
		double GetRadius() const {
			if (GetSuperType() == SUPERTYPE_STAR)
				return radius.ToDouble() * SOL_RADIUS;
			else
				return radius.ToDouble() * EARTH_RADIUS;
		}
		double GetMass() const {
			if (GetSuperType() == SUPERTYPE_STAR)
				return mass.ToDouble() * SOL_MASS;
			else
				return mass.ToDouble() * EARTH_MASS;
		}
		// returned in seconds
		double GetRotationPeriod() const {
			return rotationPeriod.ToDouble()*60*60*24;
		}

		int tmp;
		Orbit orbit;
		int seed; // Planet.cpp can use to generate terrain
		std::string name;
		fixed radius; 
		fixed mass; // earth masses if planet, solar masses if star
		fixed orbMin, orbMax; // periapsism, apoapsis in AUs
		fixed rotationPeriod; // in days
		int averageTemp;
		BodyType type;
	private:
	};
	
	SBody *rootBody;
private:
	void MakeRandomStar(SBody *sbody, MTRand &rand);
	void MakeRandomStarLighterThan(SBody *sbody, fixed maxMass, MTRand &rand);
	void MakeStarOfType(SBody *sbody, BodyType type, MTRand &rand);
	void MakeBinaryPair(SBody *a, SBody *b, fixed minDist, MTRand &rand);
	void CustomGetKidsOf(SBody *parent, const CustomSBody *customDef, const int parentIdx);
	void GenerateFromCustom(const CustomSBody *);

	int m_secx, m_secy, m_sysIdx;

	MTRand rand;
};

#endif /* _STARSYSTEM_H */
