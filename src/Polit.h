#ifndef _POLIT_H
#define _POLIT_H

#include "EquipType.h"

class StarSystem;

namespace Polit {
	enum Crime {
		CRIME_TRADING_ILLEGAL_GOODS = (1<<0)
	};

	enum Alignment {
		POL_INVALID,
		POL_NONE,
		POL_EARTH,
		POL_CONFED,
		POL_MAX
	};
	Polit::Alignment GetAlignmentForStarSystem(StarSystem *s, fixed human_infestedness);
	bool IsCommodityLegal(StarSystem *s, Equip::Type t);
	void Init();
	void Serialize();
	void Unserialize();
	void AddCrime(Sint64 crimeBitset, Sint64 addFine);
	void GetCrime(Sint64 *crimeBitset, Sint64 *fine);
	const char *GetDesc(StarSystem *s);
}

#endif /* _POLIT_H */
