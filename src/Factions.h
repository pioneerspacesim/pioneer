#ifndef _FACTIONS_H
#define _FACTIONS_H

#include "galaxy/StarSystem.h"
#include "Polit.h"
#include "vector3.h"
#include "fixed.h"

class Faction {
public:
	typedef std::map<std::string, Faction*> FactionMap;
	static void Init();
	static void Uninit();

	// XXX this is not as const-safe as it should be
	static const Faction *GetFaction(const std::string& nameIdx);

	Faction();
	~Faction();

	std::string				name;				// Formal name "Federation", "Empire", "Bob's Rib-shack consortium of delicious worlds (tm)", etc.
	std::string				description_short;	// short description
	std::string				description;		// detailed description describing formation, current status, etc
	Polit::GovType			govType;
	bool					hasHomeworld;
	SystemPath				homeworld;			// sector(x,y,x) + system index + body index = location in a (custom?) system of homeworld
	double					foundingDate;		// date faction came into existence
	double					expansionRate;		// lightyears per year that the volume expands.
	std::string				military_name;		// "Space Defense Force", "Imperial Will Enforcement Division"...
	//military logo
	std::string				police_name;		// "Police", "Polizia Locale"...
	//police logo
	//goods/equipment availability (1-per-economy-type: aka agricultural, industrial, tourist, etc)
	static const int		SC_NUM_ECONOMY_TYPES = 3;
	EquipType				types[SC_NUM_ECONOMY_TYPES][Equip::TYPE_MAX];
	//goods/equipment legality
	//ship availability
};

#endif /* _FACTIONS_H */
