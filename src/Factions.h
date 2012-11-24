// Copyright © 2008-2012 Pioneer Developers. See AUTHORS.txt for details
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
	static void Uninit();

	// XXX this is not as const-safe as it should be
	static Faction *GetFaction(const Uint32 index);
	static const Uint32 GetNumFactions();

	static const Color  GetNearestFactionColour(const Sector sec, Uint32 sysIndex);
	static const Uint32 GetNearestFactionIndex(const Sector sec, Uint32 sysIndex);
	static const Uint32 GetNearestFactionIndex(const SystemPath& sysPath);
	
	static const Uint32 GetIndexOfFaction(const std::string factionName);

	static const Uint32 BAD_FACTION_IDX;		// returned by GetNearestFactionIndex if system has no faction
	static const Color  BAD_FACTION_COLOUR;		// factionColour to use on failing to find an appropriate faction
	static const float  FACTION_BASE_ALPHA;	    // Alpha to use on factionColour of systems with unknown population
		
	Faction();
	~Faction();

	std::string				name;				// Formal name "Federation", "Empire", "Bob's Rib-shack consortium of delicious worlds (tm)", etc.
	std::string				description_short;	// short description
	std::string				description;		// detailed description describing formation, current status, etc	
	
	// government types with weighting		
	typedef std::pair<Polit::GovType, int32_t> GovWeight;
	typedef std::vector<GovWeight>             GovWeightVec;
	typedef GovWeightVec::const_iterator       GovWeightIterator;
	GovWeightVec            govtype_weights;
	int32_t                 govtype_weights_total;
	
	bool					hasHomeworld;
	SystemPath				homeworld;			// sector(x,y,x) + system index + body index = location in a (custom?) system of homeworld
	double					foundingDate;		// date faction came into existence
	double					expansionRate;		// lightyears per year that the volume expands.
	std::string				military_name;		// "Space Defense Force", "Imperial Will Enforcement Division"...
	//military logo
	std::string				police_name;		// "Police", "Polizia Locale"...
	
	//police logo
	//goods/equipment availability (1-per-economy-type: aka agricultural, industrial, tourist, etc)
	//static const int		SC_NUM_ECONOMY_TYPES = 3;
	//EquipType				types[SC_NUM_ECONOMY_TYPES][Equip::TYPE_MAX];
	
	//goods/equipment legality
	typedef std::map<Equip::Type, uint32_t> EquipProbMap;
	EquipProbMap			equip_legality;
	
	//ship availability
	Color					colour;

	const double Radius() const { return (FACTION_CURRENT_YEAR - foundingDate) * expansionRate; };
	const Polit::GovType PickGovType(MTRand &rand) const;

	// set the homeworld to one near the supplied co-ordinates
	void SetBestFitHomeworld(Sint32 x, Sint32 y, Sint32 z, Sint32 si, Uint32 bi);

private:
	static const double FACTION_CURRENT_YEAR;	// used to calculate faction radius	

	Sector* m_homesector;						// cache of home sector to use in distance calculations
	const bool IsCloserAndContains(double& closestFactionDist, const Sector sec, Uint32 sysIndex);
};

#endif /* _FACTIONS_H */
