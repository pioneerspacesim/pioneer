#ifndef _STARSYSTEM_H
#define _STARSYSTEM_H

#include "libs.h"
#include "EquipType.h"
#include "Polit.h"
#include "SysLoc.h"
#include "Serializer.h"
#include <vector>
#include <string>
#include "mylua.h"

struct CustomSBody;
struct CustomSystem;
class SBody;

// doubles: all masses in Kg, all lengths in meters
// fixed: any mad scheme

enum {  ECON_MINING = (1<<0), 
	ECON_AGRICULTURE = (1<<1), 
	ECON_INDUSTRY = (1<<2) };	

class StarSystem;

struct Orbit {
	vector3d OrbitalPosAtTime(double t);
	// 0.0 <= t <= 1.0. Not for finding orbital pos
	vector3d EvenSpacedPosAtTime(double t);
	/* duplicated from SBody... should remove probably */
	double eccentricity;
	double semiMajorAxis;
	/* dup " " --------------------------------------- */
	double period; // seconds
	matrix4x4d rotMatrix;
};

#define SBODYPATHLEN	8

class SBodyPath: public SysLoc {
public:
	SBodyPath();
	SBodyPath(int sectorX, int sectorY, int systemNum);
	Sint8 elem[SBODYPATHLEN];
	
	void Serialize(Serializer::Writer &wr) const;
	static void Unserialize(Serializer::Reader &rd, SBodyPath *path);
	
	bool operator== (const SBodyPath b) const {
		for (int i=0; i<SBODYPATHLEN; i++) if (elem[i] != b.elem[i]) return false;
		return (sectorX == b.sectorX) && (sectorY == b.sectorY) && (systemNum == b.systemNum);
	}
	/** These are for the Lua wrappers -- best not to use them from C++
	 * since they acquire a StarSystem object in a rather sub-optimal way */
	const char *GetBodyName() const;
	Uint32 GetSeed() const;
	SysLoc GetSystem() const { return (SysLoc)*this; }
private:
	/** Returned SBody only valid pointer for duration described in
	 * StarSystem::GetCached comment */
	const SBody *GetSBody() const;
};

OOLUA_CLASS(SBodyPath): public Proxy_class<SysLoc>
	OOLUA_BASIC
	OOLUA_TYPEDEFS
		OOLUA::Equal_op
	OOLUA_END_TYPES
	OOLUA_ONLY_DEFAULT_CONSTRUCTOR
	OOLUA_BASES_START SysLoc OOLUA_BASES_END
	OOLUA_MEM_FUNC_0_CONST(const char *, GetBodyName)
	OOLUA_MEM_FUNC_0_CONST(Uint32, GetSeed)
	OOLUA_MEM_FUNC_0_CONST(SysLoc, GetSystem);
OOLUA_CLASS_END

class SBody {
public:
	SBody();
	~SBody();
	void PickPlanetType(StarSystem *, MTRand &rand);
	const SBody *FindStarAndTrueOrbitalRange(fixed &orbMin, fixed &orbMax);
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
		TYPE_PLANET_DESERT,
		TYPE_PLANET_CO2,
		TYPE_PLANET_METHANE,
		TYPE_PLANET_WATER_THICK_ATMOS,
		TYPE_PLANET_CO2_THICK_ATMOS,
		TYPE_PLANET_METHANE_THICK_ATMOS,
		TYPE_PLANET_HIGHLY_VOLCANIC,
		TYPE_PLANET_INDIGENOUS_LIFE,
		TYPE_PLANET_TERRAFORMED_POOR,
		TYPE_PLANET_TERRAFORMED_GOOD,
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
	void PopulateStage1(StarSystem *system, fixed &outTotalPop);
	void PopulateAddStations(StarSystem *system);

	int tmp;
	Orbit orbit;
	Uint32 seed; // Planet.cpp can use to generate terrain
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
	
	/* economy type stuff */
	fixed m_population;
	fixed m_metallicity;
	fixed m_agricultural;

	const char *heightMapFilename;

private:
};

class StarSystem {
public:
	friend class SBody;
	StarSystem() { rootBody = 0; }
	StarSystem(int sector_x, int sector_y, int system_idx);
	~StarSystem();
	/** Holding pointers to StarSystem returned by this is not safe between
	 * calls to ShrinkCache() (done at end of Pi main loop)
	 */
	static StarSystem *GetCached(const SysLoc &s);
	static void ShrinkCache();

	const std::string &GetName() const { return m_name; }
	void GetPathOf(const SBody *body, SBodyPath *path) const;
	SBody *GetBodyByPath(const SBodyPath *path) const;
	static void Serialize(Serializer::Writer &wr, StarSystem *);
	static StarSystem *Unserialize(Serializer::Reader &rd);
	void Dump();
	bool IsSystem(int sector_x, int sector_y, int system_idx);
	int SectorX() const { return m_loc.sectorX; }
	int SectorY() const { return m_loc.sectorY; }
	int SystemIdx() const { return m_loc.systemNum; }
	const SysLoc &GetLocation() const { return m_loc; }
	void GetPos(int *sec_x, int *sec_y, int *sys_idx) const {
		*sec_x = m_loc.sectorX; *sec_y = m_loc.sectorY; *sys_idx = m_loc.systemNum;
	}
	const char *GetShortDescription() const { return m_shortDesc.c_str(); }
	const char *GetLongDescription() const { return m_longDesc.c_str(); }
	int GetNumStars() const { return m_numStars; }
	bool GetRandomStarportNearButNotIn(MTRand &rand, SBodyPath *outDest) const;
	const SysPolit &GetSysPolit() const { return m_polit; }

	static float starColors[][3];
	static float starRealColors[][3];
	static float starLuminosities[];

	struct BodyStats {

	};

	SBody *rootBody;
	std::vector<SBody*> m_spaceStations;
	
	fixed m_metallicity;
	fixed m_industrial;
	fixed m_agricultural;

	fixed m_humanProx;
	fixed m_totalPop;
	// percent price alteration
	int m_tradeLevel[Equip::TYPE_MAX];
	int m_econType;
	int m_techlevel; /* 0-5 like in EquipType.h */
	int m_seed;
	
	int GetCommodityBasePriceModPercent(int t) {
		return m_tradeLevel[t];
	}
private:
	void MakeShortDescription(MTRand &rand);
	void MakePlanetsAround(SBody *primary, MTRand &rand);
	void MakeRandomStar(SBody *sbody, MTRand &rand);
	void MakeStarOfType(SBody *sbody, SBody::BodyType type, MTRand &rand);
	void MakeStarOfTypeLighterThan(SBody *sbody, SBody::BodyType type, fixed maxMass, MTRand &rand);
	void MakeBinaryPair(SBody *a, SBody *b, fixed minDist, MTRand &rand);
	void CustomGetKidsOf(SBody *parent, const CustomSBody *customDef, const int parentIdx, int *outHumanInfestedness, MTRand &rand);
	void GenerateFromCustom(const CustomSystem *, MTRand &rand);
	void Populate(bool addSpaceStations);

	SysLoc m_loc;
	int m_numStars;
	std::string m_name;
	std::string m_shortDesc, m_longDesc;
	SysPolit m_polit;
};
	
#endif /* _STARSYSTEM_H */
