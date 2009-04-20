#ifndef _STARSYSTEM_H
#define _STARSYSTEM_H

#include "libs.h"
#include "EquipType.h"
#include <vector>
#include <string>

struct CustomSBody;

// doubles: all masses in Kg, all lengths in meters
// fixed: any mad scheme

enum {  ECON_MINING = (1<<0), 
	ECON_AGRICULTURE = (1<<1), 
	ECON_INDUSTRY = (1<<2) };	

class StarSystem;

struct Orbit {
	void KeplerPosAtTime(double t, double *dist, double *ang);
	vector3d CartesianPosAtTime(double t);
	/* duplicated from SBody... should remove probably */
	double eccentricity;
	double semiMajorAxis;
	/* dup " " --------------------------------------- */
	double period; // seconds
	matrix4x4d rotMatrix;
};

#define SBODYPATHLEN	8

struct SBodyPath {
	SBodyPath();
	SBodyPath(int sectorX, int sectorY, int systemIdx);
	int sectorX, sectorY, systemIdx;
	Sint8 elem[SBODYPATHLEN];
	
	void Serialize() const;
	static void Unserialize(SBodyPath *path);
};

class SBody {
public:
	SBody();
	~SBody();
	void EliminateBadChildren();
	void PickPlanetType(StarSystem *, SBody *, fixed distToPrimary, MTRand &drand, bool genMoons);
	SBody *parent;
	std::vector<SBody*> children;

	enum BodyType {
		TYPE_GRAVPOINT,
		TYPE_BROWN_DWARF,
		TYPE_STAR_M,
		TYPE_STAR_K,
		TYPE_WHITE_DWARF,
		TYPE_STAR_G,
		TYPE_STAR_F,
		TYPE_STAR_A,
		TYPE_STAR_B,
		TYPE_STAR_O,
		TYPE_PLANET_SMALL_GAS_GIANT,
		TYPE_PLANET_MEDIUM_GAS_GIANT,
		TYPE_PLANET_LARGE_GAS_GIANT,
		TYPE_PLANET_VERY_LARGE_GAS_GIANT,
		/* yeah yeah, asteroids aren't planets technically... */
		TYPE_PLANET_ASTEROID,
		TYPE_PLANET_LARGE_ASTEROID,
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
		TYPE_STARPORT_ORBITAL,
		TYPE_STARPORT_SURFACE,
		TYPE_MAX,
		TYPE_STAR_MIN = TYPE_BROWN_DWARF,
		TYPE_STAR_MAX = TYPE_STAR_O
		// XXX need larger atmosphereless thing
	};
	
	enum BodySuperType {
		SUPERTYPE_NONE, SUPERTYPE_STAR, SUPERTYPE_ROCKY_PLANET, SUPERTYPE_GAS_GIANT, SUPERTYPE_STARPORT
	};

	const char *GetAstroDescription();
	const char *GetIcon();
	BodySuperType GetSuperType() const;
	double GetRadius() const {
		if (GetSuperType() <= SUPERTYPE_STAR)
			return radius.ToDouble() * SOL_RADIUS;
		else
			return radius.ToDouble() * EARTH_RADIUS;
	}
	double GetMass() const {
		if (GetSuperType() <= SUPERTYPE_STAR)
			return mass.ToDouble() * SOL_MASS;
		else
			return mass.ToDouble() * EARTH_MASS;
	}
	fixed GetMassInEarths() const {
		if (GetSuperType() <= SUPERTYPE_STAR)
			return mass * 332998;
		else
			return mass;
	}

	// returned in seconds
	double GetRotationPeriod() const {
		return rotationPeriod.ToDouble()*60*60*24;
	}
	fixed CalcHillRadius() const;

	double GetMaxChildOrbitalDistance() const;
	void AddHumanStuff(StarSystem *system);

	int tmp;
	Orbit orbit;
	int seed; // Planet.cpp can use to generate terrain
	std::string name;
	fixed radius; 
	fixed mass; // earth masses if planet, solar masses if star
	fixed orbMin, orbMax; // periapsism, apoapsis in AUs
	fixed rotationPeriod; // in days
	fixed humanActivity; // 0 - 1
	fixed semiMajorAxis; // in AUs
	fixed eccentricity;
	int averageTemp;
	BodyType type;

	// percent price alteration
	int tradeLevel[Equip::TYPE_MAX];
	int econType;
private:
};
	
class StarSystem {
public:
	friend class SBody;
	StarSystem() { rootBody = 0; }
	StarSystem(int sector_x, int sector_y, int system_idx);
	~StarSystem();
	void GetPathOf(const SBody *body, SBodyPath *path) const;
	SBody *GetBodyByPath(const SBodyPath *path) const;
	static void Serialize(StarSystem *);
	static StarSystem *Unserialize();
	bool IsSystem(int sector_x, int sector_y, int system_idx);
	void GetPos(int *sec_x, int *sec_y, int *sys_idx) {
		*sec_x = m_secx; *sec_y = m_secy; *sys_idx = m_sysIdx;
	}
	int GetNumStars() const { return m_numStars; }

	static float starColors[][3];
	static float starRealColors[][3];


	struct BodyStats {

	};

	SBody *rootBody;
	fixed m_humanInfested; // 0 to 1
private:
	void MakePlanetsAround(SBody *primary);
	void MakeRandomStar(SBody *sbody, MTRand &rand);
	void MakeStarOfType(SBody *sbody, SBody::BodyType type, MTRand &rand);
	void MakeStarOfTypeLighterThan(SBody *sbody, SBody::BodyType type, fixed maxMass, MTRand &rand);
	void MakeBinaryPair(SBody *a, SBody *b, fixed minDist, MTRand &rand);
	void CustomGetKidsOf(SBody *parent, const CustomSBody *customDef, const int parentIdx);
	void GenerateFromCustom(const CustomSBody *);
	void PickEconomicStuff(SBody *b);

	int m_secx, m_secy, m_sysIdx;
	int m_numStars;

	MTRand rand;
};


	
#endif /* _STARSYSTEM_H */
