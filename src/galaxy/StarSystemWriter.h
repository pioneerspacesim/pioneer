// Copyright © 2008-2019 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#ifndef STARSYSTEMWRITER_H
#define STARSYSTEMWRITER_H

#include "StarSystem.h"

/*
	A simple wrapper class which allow to
	access and write inside a StarSystem

	Intended only for temporary usage
*/

class StarSystemWriter {
public:
	StarSystemWriter(StarSystem *ss) : m_ssys(ss)
	{ assert(m_ssys); }

	StarSystemWriter(RefCountedPtr<StarSystem> ss) : m_ssys(ss.Get())
	{ assert(m_ssys); }

	bool HasCustomBodies() const { return m_ssys->m_hasCustomBodies; }

	void SetCustom(bool isCustom, bool hasCustomBodies)
	{
		m_ssys->m_isCustom = isCustom;
		m_ssys->m_hasCustomBodies = hasCustomBodies;
	}
	void SetNumStars(int numStars) { m_ssys->m_numStars = numStars; }
	void SetRootBody(RefCountedPtr<SystemBody> rootBody) { m_ssys->m_rootBody = rootBody; }
	void SetRootBody(SystemBody *rootBody) { m_ssys->m_rootBody.Reset(rootBody); }
	void SetName(const std::string &name) { m_ssys->m_name = name; }
	void SetOtherNames(const std::vector<std::string> &other_names) {m_ssys-> m_other_names = other_names; }
	void SetLongDesc(const std::string &desc) { m_ssys->m_longDesc = desc; }
	void SetExplored(ExplorationState explored, double time)
	{
		m_ssys->m_explored = explored;
		m_ssys->m_exploredTime = time;
	}
	void SetSeed(Uint32 seed) { m_ssys->m_seed = seed; }
	void SetFaction(const Faction *faction) { m_ssys->m_faction = faction; }
	void SetEconType(GalacticEconomy::EconType econType) { m_ssys->m_econType = econType; }
	void SetSysPolit(SysPolit polit) { m_ssys->m_polit = polit; }
	void SetMetallicity(fixed metallicity) { m_ssys->m_metallicity = metallicity; }
	void SetIndustrial(fixed industrial) { m_ssys->m_industrial = industrial; }
	void SetAgricultural(fixed agricultural) { m_ssys->m_agricultural = agricultural; }
	void SetHumanProx(fixed humanProx) { m_ssys->m_humanProx = humanProx; }
	void SetTotalPop(fixed pop) { m_ssys->m_totalPop = pop; }
	void AddTotalPop(fixed pop) { m_ssys->m_totalPop += pop; }
	void SetTradeLevel(GalacticEconomy::Commodity type, int level) { m_ssys->m_tradeLevel[int(type)] = level; }
	void AddTradeLevel(GalacticEconomy::Commodity type, int level) { m_ssys->m_tradeLevel[int(type)] += level; }
	void SetCommodityLegal(GalacticEconomy::Commodity type, bool legal) { m_ssys->m_commodityLegal[int(type)] = legal; }

	void AddSpaceStation(SystemBody *station)
	{
		assert(station->GetSuperType() == SystemBody::SUPERTYPE_STARPORT);
		m_ssys->m_spaceStations.push_back(station);
	}
	void AddStar(SystemBody *star)
	{
		assert(star->GetSuperType() == SystemBody::SUPERTYPE_STAR);
		m_ssys->m_stars.push_back(star);
	}

	void SetShortDesc(const std::string &desc) { m_ssys->m_shortDesc = desc; }

	void MakeShortDescription() { m_ssys->MakeShortDescription(); };

private:
	StarSystem *m_ssys;
};

#endif // STARSYSTEMWRITER_H
