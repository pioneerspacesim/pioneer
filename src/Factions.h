// Copyright © 2008-2014 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#ifndef _FACTIONS_H
#define _FACTIONS_H

#include "galaxy/Sector.h"
#include "galaxy/StarSystem.h"
#include "Polit.h"
#include "vector3.h"
#include "fixed.h"
#include "DeleteEmitter.h"
#include <map>
#include <vector>
#include <utility>

class Faction : public DeleteEmitter {
public:
	static void Init();
	static void ClearHomeSectors();
	static void SetHomeSectors();
	static void Uninit();

	// XXX this is not as const-safe as it should be
	static Faction *GetFaction       (const Uint32 index);
	static Faction *GetFaction       (const std::string& factionName);
	static Faction *GetNearestFaction(RefCountedPtr<const Sector> sec, Uint32 sysIndex);
	static bool     IsHomeSystem     (const SystemPath& sysPath);

	static const Uint32 GetNumFactions();

	static bool MayAssignFactions();

	static const Uint32 BAD_FACTION_IDX;        // used by the no faction object to denote it's not a proper faction
	static const Color  BAD_FACTION_COLOUR;     // factionColour to use on failing to find an appropriate faction
	static const float  FACTION_BASE_ALPHA;     // Alpha to use on factionColour of systems with unknown population

	Faction();

	Uint32             idx;                 // faction index
	std::string	       name;                // Formal name "Federation", "Empire", "Bob's Rib-shack consortium of delicious worlds (tm)", etc.
	std::string	       description_short;   // short description
	std::string        description;         // detailed description describing formation, current status, etc

	// government types with weighting
	typedef std::pair<Polit::GovType, Sint32> GovWeight;
	typedef std::vector<GovWeight>            GovWeightVec;
	typedef GovWeightVec::const_iterator      GovWeightIterator;
	GovWeightVec       govtype_weights;
	Sint32             govtype_weights_total;

	bool               hasHomeworld;
	SystemPath         homeworld;           // sector(x,y,x) + system index + body index = location in a (custom?) system of homeworld
	double             foundingDate;        // date faction came into existence
	double             expansionRate;       // lightyears per year that the volume expands.
	std::string        military_name;       // "Space Defense Force", "Imperial Will Enforcement Division"...
	//military logo
	std::string        police_name;         // "Police", "Polizia Locale"...

	//police logo
	//goods/equipment availability (1-per-economy-type: aka agricultural, industrial, tourist, etc)
	//static const int		SC_NUM_ECONOMY_TYPES = 3;
	//EquipType				types[SC_NUM_ECONOMY_TYPES][Equip::TYPE_MAX];

	//goods/equipment legality
	typedef std::map<Equip::Type, Uint32> EquipProbMap;
	EquipProbMap       equip_legality;

	Color              colour;

	const double         Radius()  const { return (FACTION_CURRENT_YEAR - foundingDate) * expansionRate; };
	const bool           IsValid() const { return idx != BAD_FACTION_IDX; };
	const Color          AdjustedColour(fixed population, bool inRange);
	const Polit::GovType PickGovType(Random &rand) const;

	// set the homeworld to one near the supplied co-ordinates
	void SetBestFitHomeworld(Sint32 x, Sint32 y, Sint32 z, Sint32 si, Uint32 bi, Sint32 axisChange);
	RefCountedPtr<const Sector> GetHomeSector();

private:
	static const double FACTION_CURRENT_YEAR;	// used to calculate faction radius

	RefCountedPtr<const Sector> m_homesector;	// cache of home sector to use in distance calculations
	const bool IsCloserAndContains(double& closestFactionDist, RefCountedPtr<const Sector> sec, Uint32 sysIndex);
};

/* One day it might grow up to become a full tree, on the  other hand it might be
   cut down before it's full growth to be replaced by
   a proper spatial data structure.
*/

class FactionOctsapling {
public:
	void Add(Faction* faction);
	const std::vector<Faction*>& CandidateFactions(RefCountedPtr<const Sector> sec, Uint32 sysIndex);

private:
	std::vector<Faction*> octbox[2][2][2];
	const int BoxIndex(Sint32 sectorIndex) { return sectorIndex < 0 ? 0: 1; };
	void PruneDuplicates(const int bx, const int by, const int bz);
};


#endif /* _FACTIONS_H */
