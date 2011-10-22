#ifndef _POLIT_H
#define _POLIT_H

#include "EquipType.h"
#include "Serializer.h"

class StarSystem;
class SysPolit;
class Ship;

namespace Polit {
	enum Crime {
#define Crime_ITEM(x,y) CRIME_##x = y,
#include "PolitEnums.h"
	};

	enum Bloc {
#define Bloc_ITEM(x) BLOC_##x,
#include "PolitEnums.h"
		BLOC_MAX
	};

	enum EconType {
#define PolitEcon_ITEM(x) ECON_##x,
#include "PolitEnums.h"
		ECON_MAX
	};

	enum GovType {
#define GovType_ITEM(x) GOV_##x,
#define GovType_ITEM_X(x) GOV_##x,
#define GovType_ITEM_Y(x,y) GOV_##x = y,
#include "PolitEnums.h"
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
