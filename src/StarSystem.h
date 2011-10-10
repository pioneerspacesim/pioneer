#ifndef _STARSYSTEM_H
#define _STARSYSTEM_H

#include "libs.h"
#include "EquipType.h"
#include "Polit.h"
#include "Serializer.h"
#include <vector>
#include <string>
#include "DeleteEmitter.h"
#include "RefCounted.h"
#include "SystemPath.h"

class CustomSBody;
class CustomSystem;
class SBody;

// doubles - all masses in Kg, all lengths in meters
// fixed - any mad scheme

enum {
#define EconType_ITEM(x,y) ECON_##x = y,
#include "StarSystemEnums.h"
};

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

class SBody {
public:
	SBody();
	~SBody();
	void PickPlanetType(StarSystem *, MTRand &rand);
	const SBody *FindStarAndTrueOrbitalRange(fixed &orbMin, fixed &orbMax);
	SBody *parent;
	std::vector<SBody*> children;

	enum BodyType {
#define BodyType_ITEM(x,y) TYPE_##x = y,
#include "StarSystemEnums.h"
	};
	
	enum BodySuperType {
#define BodySuperType_ITEM(x,y) SUPERTYPE_##x = y,
#include "StarSystemEnums.h"
	};

	std::string GetAstroDescription();
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

	Uint32 id; // index into starsystem->m_bodies
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
	fixed orbitalOffset;
	fixed axialTilt; // in radians
	int averageTemp;
	BodyType type;

	/* composition */
	fixed m_metallicity; // (crust) 0.0 = light (Al, SiO2, etc), 1.0 = heavy (Fe, heavy metals)
	fixed m_volatileGas; // 1.0 = earth atmosphere density
	fixed m_volatileLiquid; // 1.0 = 100% ocean cover (earth = 70%)
	fixed m_volatileIces; // 1.0 = 100% ice cover (earth = 3%)
	fixed m_volcanicity; // 0 = none, 1.0 = fucking volcanic
	fixed m_atmosOxidizing; // 0.0 = reducing (H2, NH3, etc), 1.0 = oxidising (CO2, O2, etc)
	fixed m_life; // 0.0 = dead, 1.0 = teeming
	
	/* economy type stuff */
	fixed m_population;
	fixed m_agricultural;

	const char *heightMapFilename;

private:
};

class StarSystem : public DeleteEmitter, public RefCounted {
public:
	friend class SBody;

	static StarSystem *GetCached(const SystemPath &path);
	inline void Release() { DecRefCount(); }
	static void ShrinkCache();

	const std::string &GetName() const { return m_name; }
	SystemPath GetPathOf(const SBody *sbody) const;
	SBody *GetBodyByPath(const SystemPath &path) const;
	static void Serialize(Serializer::Writer &wr, StarSystem *);
	static StarSystem *Unserialize(Serializer::Reader &rd);
	void Dump();
	const SystemPath &GetPath() const { return m_path; }
	const char *GetShortDescription() const { return m_shortDesc.c_str(); }
	const char *GetLongDescription() const { return m_longDesc.c_str(); }
	int GetNumStars() const { return m_numStars; }
	const SysPolit &GetSysPolit() const { return m_polit; }

	static float starColors[][3];
	static float starRealColors[][3];
	static double starLuminosities[];
	static float starScale[];
	static fixed starMetallicities[];

	SBody *rootBody;
	std::vector<SBody*> m_spaceStations;
	// index into this will be the SBody ID used by SystemPath
	std::vector<SBody*> m_bodies;
	
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

	bool m_unexplored;
	
	int GetCommodityBasePriceModPercent(int t) {
		return m_tradeLevel[t];
	}
private:
	StarSystem(const SystemPath &path);
	~StarSystem();

	SBody *NewBody() {
		SBody *body = new SBody;
		body->id = m_bodies.size();
		m_bodies.push_back(body);
		return body;
	}
	void MakeShortDescription(MTRand &rand);
	void MakePlanetsAround(SBody *primary, MTRand &rand);
	void MakeRandomStar(SBody *sbody, MTRand &rand);
	void MakeStarOfType(SBody *sbody, SBody::BodyType type, MTRand &rand);
	void MakeStarOfTypeLighterThan(SBody *sbody, SBody::BodyType type, fixed maxMass, MTRand &rand);
	void MakeBinaryPair(SBody *a, SBody *b, fixed minDist, MTRand &rand);
	void CustomGetKidsOf(SBody *parent, const std::list<CustomSBody> *children, int *outHumanInfestedness, MTRand &rand);
	void GenerateFromCustom(const CustomSystem *, MTRand &rand);
	void Populate(bool addSpaceStations);

	SystemPath m_path;
	int m_numStars;
	std::string m_name;
	std::string m_shortDesc, m_longDesc;
	SysPolit m_polit;

	bool m_isCustom;
	bool m_hasCustomBodies;
};
	
#endif /* _STARSYSTEM_H */
