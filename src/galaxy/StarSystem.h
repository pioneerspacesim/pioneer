// Copyright © 2008-2019 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#ifndef _STARSYSTEM_H
#define _STARSYSTEM_H

#include "../RefCounted.h"
#include "galaxy/Economy.h"
#include "galaxy/GalaxyCache.h"
#include "galaxy/Polit.h"
#include "galaxy/SystemBody.h"
#include "galaxy/SystemPath.h"
#include "gameconsts.h"

#include <SDL_stdinc.h>
#include <string>
#include <vector>

class Faction;
class Galaxy;
class CustomSystemBody;
class CustomSystem;

// doubles - all masses in Kg, all lengths in meters
// fixed - any mad scheme

class StarSystem : public RefCounted {
public:
	class GeneratorAPI; // Complete definition below

	enum ExplorationState {
		eUNEXPLORED = 0,
		eEXPLORED_BY_PLAYER = 1,
		eEXPLORED_AT_START = 2
	};

	void ExportToLua(const char *filename);

	const std::string &GetName() const { return m_name; }
	std::vector<std::string> GetOtherNames() const { return m_other_names; }
	SystemPath GetPathOf(const SystemBody *sbody) const;
	SystemBody *GetBodyByPath(const SystemPath &path) const;
	static void ToJson(Json &jsonObj, StarSystem *);
	static RefCountedPtr<StarSystem> FromJson(RefCountedPtr<Galaxy> galaxy, const Json &jsonObj);
	const SystemPath &GetPath() const { return m_path; }
	const std::string &GetShortDescription() const { return m_shortDesc; }
	const std::string &GetLongDescription() const { return m_longDesc; }
	unsigned GetNumStars() const { return m_numStars; }
	const SysPolit &GetSysPolit() const { return m_polit; }

	static const Color starColors[];
	static const Color starRealColors[];
	static const double starLuminosities[];
	static const float starScale[];

	RefCountedPtr<const SystemBody> GetRootBody() const { return m_rootBody; }
	RefCountedPtr<SystemBody> GetRootBody() { return m_rootBody; }
	bool HasSpaceStations() const { return !m_spaceStations.empty(); }
	Uint32 GetNumSpaceStations() const { return static_cast<Uint32>(m_spaceStations.size()); }
	IterationProxy<std::vector<SystemBody *>> GetSpaceStations() { return MakeIterationProxy(m_spaceStations); }
	const IterationProxy<const std::vector<SystemBody *>> GetSpaceStations() const { return MakeIterationProxy(m_spaceStations); }
	IterationProxy<std::vector<SystemBody *>> GetStars() { return MakeIterationProxy(m_stars); }
	const IterationProxy<const std::vector<SystemBody *>> GetStars() const { return MakeIterationProxy(m_stars); }
	Uint32 GetNumBodies() const { return static_cast<Uint32>(m_bodies.size()); }
	IterationProxy<std::vector<RefCountedPtr<SystemBody>>> GetBodies() { return MakeIterationProxy(m_bodies); }
	const IterationProxy<const std::vector<RefCountedPtr<SystemBody>>> GetBodies() const { return MakeIterationProxy(m_bodies); }

	bool IsCommodityLegal(const GalacticEconomy::Commodity t)
	{
		return m_commodityLegal[int(t)];
	}

	int GetCommodityBasePriceModPercent(GalacticEconomy::Commodity t)
	{
		return m_tradeLevel[int(t)];
	}

	const Faction *GetFaction() const { return m_faction; }
	bool GetUnexplored() const { return m_explored == eUNEXPLORED; }
	ExplorationState GetExplored() const { return m_explored; }
	double GetExploredTime() const { return m_exploredTime; }
	void ExploreSystem(double time);

	fixed GetMetallicity() const { return m_metallicity; }
	fixed GetIndustrial() const { return m_industrial; }
	fixed GetAgricultural() const { return m_agricultural; }
	GalacticEconomy::EconType GetEconType() const { return m_econType; }
	const int *GetTradeLevel() const { return m_tradeLevel; }
	int GetSeed() const { return m_seed; }
	fixed GetHumanProx() const { return m_humanProx; }
	fixed GetTotalPop() const { return m_totalPop; }

	void Dump(FILE *file, const char *indent = "", bool suppressSectorData = false) const;

	const RefCountedPtr<Galaxy> m_galaxy;

	void SetCache(StarSystemCache *cache)
	{
		assert(!m_cache);
		m_cache = cache;
	}

protected:
	StarSystem(const SystemPath &path, RefCountedPtr<Galaxy> galaxy, StarSystemCache *cache, Random &rand);
	virtual ~StarSystem();

	SystemBody *NewBody()
	{
		SystemBody *body = new SystemBody(SystemPath(m_path.sectorX, m_path.sectorY, m_path.sectorZ, m_path.systemIndex, static_cast<Uint32>(m_bodies.size())), this);
		m_bodies.push_back(RefCountedPtr<SystemBody>(body));
		return body;
	}

	void MakeShortDescription();
	void SetShortDesc(const std::string &desc) { m_shortDesc = desc; }

private:
	std::string ExportBodyToLua(FILE *f, SystemBody *body);
	std::string GetStarTypes(SystemBody *body);

	SystemPath m_path;
	unsigned m_numStars;
	std::string m_name;
	std::vector<std::string> m_other_names;
	std::string m_shortDesc, m_longDesc;
	SysPolit m_polit;

	bool m_isCustom;
	bool m_hasCustomBodies;

	const Faction *m_faction;
	ExplorationState m_explored;
	double m_exploredTime;
	fixed m_metallicity;
	fixed m_industrial;
	GalacticEconomy::EconType m_econType;
	Uint32 m_seed;

	// percent price alteration
	int m_tradeLevel[GalacticEconomy::COMMODITY_COUNT];

	fixed m_agricultural;
	fixed m_humanProx;
	fixed m_totalPop;

	RefCountedPtr<SystemBody> m_rootBody;
	// index into this will be the SystemBody ID used by SystemPath
	std::vector<RefCountedPtr<SystemBody>> m_bodies;
	std::vector<SystemBody *> m_spaceStations;
	std::vector<SystemBody *> m_stars;
	std::vector<bool> m_commodityLegal;

	StarSystemCache *m_cache;
};

class StarSystem::GeneratorAPI : public StarSystem {
private:
	friend class GalaxyGenerator;
	GeneratorAPI(const SystemPath &path, RefCountedPtr<Galaxy> galaxy, StarSystemCache *cache, Random &rand);

public:
	bool HasCustomBodies() const { return m_hasCustomBodies; }

	void SetCustom(bool isCustom, bool hasCustomBodies)
	{
		m_isCustom = isCustom;
		m_hasCustomBodies = hasCustomBodies;
	}
	void SetNumStars(int numStars) { m_numStars = numStars; }
	void SetRootBody(RefCountedPtr<SystemBody> rootBody) { m_rootBody = rootBody; }
	void SetRootBody(SystemBody *rootBody) { m_rootBody.Reset(rootBody); }
	void SetName(const std::string &name) { m_name = name; }
	void SetOtherNames(const std::vector<std::string> &other_names) { m_other_names = other_names; }
	void SetLongDesc(const std::string &desc) { m_longDesc = desc; }
	void SetExplored(ExplorationState explored, double time)
	{
		m_explored = explored;
		m_exploredTime = time;
	}
	void SetSeed(Uint32 seed) { m_seed = seed; }
	void SetFaction(const Faction *faction) { m_faction = faction; }
	void SetEconType(GalacticEconomy::EconType econType) { m_econType = econType; }
	void SetSysPolit(SysPolit polit) { m_polit = polit; }
	void SetMetallicity(fixed metallicity) { m_metallicity = metallicity; }
	void SetIndustrial(fixed industrial) { m_industrial = industrial; }
	void SetAgricultural(fixed agricultural) { m_agricultural = agricultural; }
	void SetHumanProx(fixed humanProx) { m_humanProx = humanProx; }
	void SetTotalPop(fixed pop) { m_totalPop = pop; }
	void AddTotalPop(fixed pop) { m_totalPop += pop; }
	void SetTradeLevel(GalacticEconomy::Commodity type, int level) { m_tradeLevel[int(type)] = level; }
	void AddTradeLevel(GalacticEconomy::Commodity type, int level) { m_tradeLevel[int(type)] += level; }
	void SetCommodityLegal(GalacticEconomy::Commodity type, bool legal) { m_commodityLegal[int(type)] = legal; }

	void AddSpaceStation(SystemBody *station)
	{
		assert(station->GetSuperType() == SystemBody::SUPERTYPE_STARPORT);
		m_spaceStations.push_back(station);
	}
	void AddStar(SystemBody *star)
	{
		assert(star->GetSuperType() == SystemBody::SUPERTYPE_STAR);
		m_stars.push_back(star);
	}
	using StarSystem::MakeShortDescription;
	using StarSystem::NewBody;
	using StarSystem::SetShortDesc;
};

#endif /* _STARSYSTEM_H */
