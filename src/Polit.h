// Copyright © 2008-2015 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#ifndef _POLIT_H
#define _POLIT_H

#include "galaxy/Economy.h"
#include "Serializer.h"
#include "json/json.h"

class Galaxy;
class StarSystem;
class SysPolit;
class Ship;

namespace Polit {
	enum Crime { // <enum scope='Polit' name=PolitCrime prefix=CRIME_ public>
		CRIME_TRADING_ILLEGAL_GOODS = (1<<0),
		CRIME_WEAPON_DISCHARGE = (1<<1),
		CRIME_PIRACY = (1<<2),
		CRIME_MURDER = (1<<3),
	};

	enum PolitEcon { // <enum scope='Polit' name=PolitEcon prefix=ECON_ public>
		ECON_NONE,
		ECON_VERY_CAPITALIST,
		ECON_CAPITALIST,
		ECON_MIXED,
		ECON_PLANNED,
		ECON_MAX // <enum skip>
	};

	enum GovType { // <enum scope='Polit' name=PolitGovType prefix=GOV_ public>
		GOV_INVALID, // <enum skip>
		GOV_NONE,
		GOV_EARTHCOLONIAL,
		GOV_EARTHDEMOC,
		GOV_EMPIRERULE,
		GOV_CISLIBDEM,
		GOV_CISSOCDEM,
		GOV_LIBDEM,
		GOV_CORPORATE,
		GOV_SOCDEM,
		GOV_EARTHMILDICT,
		GOV_MILDICT1,
		GOV_MILDICT2,
		GOV_EMPIREMILDICT,
		GOV_COMMUNIST,
		GOV_PLUTOCRATIC,
		GOV_DISORDER,
		GOV_MAX, // <enum skip>
		GOV_RAND_MIN = GOV_NONE+1, // <enum skip>
		GOV_RAND_MAX = GOV_MAX-1, // <enum skip>
	};

	void NotifyOfCrime(Ship *s, enum Crime c);
	void Init(RefCountedPtr<Galaxy> galaxy);
	void ToJson(Json::Value &jsonObj);
	void FromJson(const Json::Value &jsonObj, RefCountedPtr<Galaxy> galaxy);
	void AddCrime(Sint64 crimeBitset, Sint64 addFine);
	void GetCrime(Sint64 *crimeBitset, Sint64 *fine);
	fixed GetBaseLawlessness(GovType gov);

	extern const char *crimeNames[64];
}

class SysPolit {
public:
	SysPolit() : govType(Polit::GOV_INVALID) { }

	const char *GetGovernmentDesc() const;
	const char *GetEconomicDesc() const;

	Polit::GovType govType;
	fixed lawlessness;
};

#endif /* _POLIT_H */
