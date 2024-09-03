// Copyright © 2008-2024 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#ifndef _FACTIONS_H
#define _FACTIONS_H

#include "DeleteEmitter.h"
#include "Polit.h"
#include "fixed.h"
#include "galaxy/Economy.h"
#include "galaxy/Sector.h"
#include "galaxy/StarSystem.h"
#include "vector3.h"
#include <map>
#include <utility>
#include <vector>

class Galaxy;
class CustomSystem;

class Faction : public DeleteEmitter {
	friend class FactionsDatabase;

public:
	static const Uint32 BAD_FACTION_IDX;   // used by the no faction object to denote it's not a proper faction
	static const Color BAD_FACTION_COLOUR; // factionColour to use on failing to find an appropriate faction
	static const float FACTION_BASE_ALPHA; // Alpha to use on factionColour of systems with unknown population

	Faction(Galaxy *galaxy);

	Uint32 idx;					   // faction index
	std::string name;			   // Formal name "Federation", "Empire", "Bob's Rib-shack consortium of delicious worlds (tm)", etc.
	std::string description_short; // short description
	std::string description;	   // detailed description describing formation, current status, etc

	// government types with weighting
	typedef std::pair<Polit::GovType, Sint32> GovWeight;
	typedef std::vector<GovWeight> GovWeightVec;
	typedef GovWeightVec::const_iterator GovWeightIterator;
	GovWeightVec govtype_weights;
	Sint32 govtype_weights_total;

	bool hasHomeworld;
	SystemPath homeworld;	   // sector(x,y,x) + system index + body index = location in a (custom?) system of homeworld
	double foundingDate;	   // date faction came into existence
	double expansionRate;	   // lightyears per year that the volume expands.
	std::string military_name; // "Space Defense Force", "Imperial Will Enforcement Division"...
	//military logo
	std::string police_name; // "Police", "Polizia Locale"...
	std::string police_ship; // "kanara", "varada"...
	//police logo
	//goods/equipment availability (1-per-economy-type: aka agricultural, industrial, tourist, etc)

	typedef std::vector<SystemPath> ClaimList;
	ClaimList m_ownedsystemlist;
	void PushClaim(SystemPath path) { m_ownedsystemlist.push_back(path); }
	bool IsClaimed(SystemPath) const;

	// commodity legality
	typedef std::map<GalacticEconomy::CommodityId, Uint32> CommodityProbMap;
	CommodityProbMap commodity_legality;

	Color colour;

	double Radius() const { return (FACTION_CURRENT_YEAR - foundingDate) * expansionRate; };
	bool IsValid() const { return idx != BAD_FACTION_IDX; };
	Color AdjustedColour(fixed population, bool shadow) const;
	Polit::GovType PickGovType(Random &rand) const;

	// set the homeworld to one near the supplied co-ordinates
	void SetBestFitHomeworld(Sint32 x, Sint32 y, Sint32 z, Sint32 si, Uint32 bi, Sint32 axisChange);
	RefCountedPtr<const Sector> GetHomeSector() const;

private:
	static const double FACTION_CURRENT_YEAR; // used to calculate faction radius

	Galaxy *const m_galaxy;							  // galaxy we are part of
	mutable RefCountedPtr<const Sector> m_homesector; // cache of home sector to use in distance calculations
	bool IsCloserAndContains(double &closestFactionDist, const Sector::System *sys) const;
};

/* One day it might grow up to become a full tree, on the  other hand it might be
   cut down before it's full growth to be replaced by
   a proper spatial data structure.
*/

class FactionsDatabase {
public:
	FactionsDatabase(Galaxy *galaxy, const std::string &factionDir) :
		m_galaxy(galaxy),
		m_factionDirectory(factionDir),
		m_no_faction(galaxy),
		m_may_assign_factions(false),
		m_initialized(false) {}
	~FactionsDatabase();

	void Init();
	void PostInit();
	void ClearCache() { ClearHomeSectors(); }
	bool IsInitialized() const;
	Galaxy *GetGalaxy() const { return m_galaxy; }
	void RegisterCustomSystem(CustomSystem *cs, const std::string &factionName);
	void AddFaction(Faction *faction);

	const Faction *GetFaction(const Uint32 index) const;
	const Faction *GetFaction(const std::string &factionName) const;
	const Faction *GetNearestClaimant(const Sector::System *sys) const;
	bool IsHomeSystem(const SystemPath &sysPath) const;

	Uint32 GetNumFactions() const;

	bool MayAssignFactions() const;

private:
	class Octsapling {
	public:
		void Add(const Faction *faction);
		const std::vector<const Faction *> &CandidateFactions(const Sector::System *sys) const;

	private:
		std::vector<const Faction *> octbox[2][2][2];
		static int BoxIndex(Sint32 sectorIndex) { return sectorIndex < 0 ? 0 : 1; };
		void PruneDuplicates(const int bx, const int by, const int bz);
	};

	typedef std::vector<Faction *> FactionList;
	typedef FactionList::iterator FactionIterator;
	typedef const std::vector<const Faction *> ConstFactionList;
	typedef ConstFactionList::const_iterator ConstFactionIterator;
	typedef std::map<std::string, Faction *> FactionMap;
	typedef std::set<SystemPath> HomeSystemSet;
	typedef std::map<std::string, std::list<CustomSystem *>> MissingFactionsMap;

	void ClearHomeSectors();
	void SetHomeSectors();

	Galaxy *const m_galaxy;
	const std::string m_factionDirectory;
	Faction m_no_faction; // instead of answering null, we often want to answer a working faction object for no faction
	FactionList m_factions;
	FactionMap m_factions_byName;
	HomeSystemSet m_homesystems;
	Octsapling m_spatial_index;
	bool m_may_assign_factions;
	bool m_initialized = false;
	MissingFactionsMap m_missingFactionsMap;
};

#endif /* _FACTIONS_H */
