#ifndef _POLIT_H
#define _POLIT_H

#include "EquipType.h"
#include "Serializer.h"

class StarSystem;
class SysPolit;
class Ship;

namespace Polit {
	enum Crime { // <enum scope='Polit' name=PolitCrime prefix=CRIME_>
		CRIME_TRADING_ILLEGAL_GOODS = (1<<0),
		CRIME_WEAPON_DISCHARGE = (1<<1),
		CRIME_PIRACY = (1<<2),
		CRIME_MURDER = (1<<3),
	};

	enum Bloc { // <enum scope='Polit' name=PolitBloc prefix=BLOC_>
		BLOC_NONE,
		BLOC_EARTHFED,
		BLOC_CIS,
		BLOC_EMPIRE,
		BLOC_MAX // <enum skip>
	};

	enum PolitEcon { // <enum scope='Polit' name=PolitEcon prefix=ECON_>
		ECON_NONE,
		ECON_VERY_CAPITALIST,
		ECON_CAPITALIST,
		ECON_MIXED,
		ECON_PLANNED,
		ECON_MAX // <enum skip>
	};

	enum GovType { // <enum scope='Polit' name=PolitGovType prefix=GOV_>
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
	void GetSysPolitStarSystem(const StarSystem *s, const fixed human_infestedness, SysPolit &outSysPolit);
	bool IsCommodityLegal(const StarSystem *s, Equip::Type t);
	void Init();
	void Serialize(Serializer::Writer &wr);
	void Unserialize(Serializer::Reader &rd);
	void AddCrime(Sint64 crimeBitset, Sint64 addFine);
	void GetCrime(Sint64 *crimeBitset, Sint64 *fine);
	/* XXX Why the hell aren't these methods on StarSystem class? */
	const char *GetGovernmentDesc(StarSystem *s);
	const char *GetEconomicDesc(StarSystem *s);
	const char *GetAllegianceDesc(StarSystem *s);

	extern const char *crimeNames[64];
}

class SysPolit {
public:
	Polit::GovType govType;
	fixed lawlessness;
};

#endif /* _POLIT_H */
