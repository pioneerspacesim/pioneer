// Copyright © 2008-2024 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#ifndef _CUSTOMSYSTEM_H
#define _CUSTOMSYSTEM_H

#include "Color.h"
#include "Polit.h"
#include "JsonFwd.h"
#include "galaxy/SystemBody.h"

#include "fixed.h"
#include "vector3.h"

class Faction;
class Galaxy;

class CustomSystemBody {
public:
	enum RingStatus {
		WANT_RANDOM_RINGS,
		WANT_RINGS,
		WANT_NO_RINGS,
		WANT_CUSTOM_RINGS
	};

	CustomSystemBody();
	~CustomSystemBody();

	SystemBodyData bodyData;
	// TODO: these two are separate implementations to handle Lua/Json based systems
	std::vector<CustomSystemBody *> children;
	std::vector<uint32_t> childIndicies;

	// TODO: these are only to be used for Lua system generation
	bool want_rand_offset;
	bool want_rand_phase;
	bool want_rand_arg_periapsis;
	bool want_rand_seed;
	RingStatus ringStatus;

	void SanityChecks();
};

class CustomSystem {
public:

	static const int CUSTOM_ONLY_RADIUS = 4;
	CustomSystem();
	~CustomSystem();

	std::string name;
	std::vector<std::string> other_names;
	uint64_t nameHash;

	CustomSystemBody *sBody;
	// TODO: this holds system body objects when loaded from Json
	// This depends on serialized body order being exactly the same as
	// depth-first hierarchy traversal order.
	// Otherwise, subtle inconsistencies and outright wrong random generation
	// will creep in.
	// The fix is to fully deprecate the depth-first traversal order and
	// "flatten" StarSystemCustomGenerator::CustomGetKidsOf
	// TODO: this should act as storage for all bodies instead of holding ptrs
	std::vector<CustomSystemBody *> bodies;
	SystemBody::BodyType primaryType[4];
	unsigned numStars;
	int sectorX, sectorY, sectorZ;
	Uint32 systemIndex;
	vector3f pos;
	Uint32 seed;
	// NOTE: these are only intended to be used for Lua system generation
	bool want_rand_seed;
	bool want_rand_explored;
	bool explored;
	const Faction *faction;
	Polit::GovType govType;
	bool want_rand_lawlessness;
	fixed lawlessness; // 0.0 = lawful, 1.0 = totally lawless
	std::string shortDesc;
	std::string longDesc;

	void SanityChecks();

	bool IsRandom() const { return !sBody; }

	void LoadFromJson(const Json &systemdef);
	void SaveToJson(Json &obj);
};

class CustomSystemsDatabase {
public:
	CustomSystemsDatabase(Galaxy *galaxy, const std::string &customSysDir) :
		m_galaxy(galaxy),
		m_customSysDirectory(customSysDir) {}
	~CustomSystemsDatabase();

	void Load();


	void LoadAllLuaSystems();
	const CustomSystem *LoadSystem(std::string_view filepath);

	CustomSystem *LoadSystemFromJSON(std::string_view filename, const Json &systemdef, bool mergeWithGalaxy = true);

	typedef std::vector<const CustomSystem *> SystemList;
	// XXX this is not as const-safe as it should be
	const SystemList &GetCustomSystemsForSector(int sectorX, int sectorY, int sectorZ) const;
	void AddCustomSystem(const SystemPath &path, CustomSystem *csys);
	Galaxy *GetGalaxy() const { return m_galaxy; }

	void RunLuaSystemSanityChecks(CustomSystem *csys);

private:
	typedef std::map<SystemPath, SystemList> SectorMap;
	typedef std::pair<SystemPath, size_t> SystemIndex;

	lua_State *CreateLoaderState();

	Galaxy *const m_galaxy;
	const std::string m_customSysDirectory;
	SectorMap m_sectorMap;
	SystemIndex m_lastAddedSystem;
	static const SystemList s_emptySystemList; // see: Null Object pattern
};

#endif /* _CUSTOMSYSTEM_H */
