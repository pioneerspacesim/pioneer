#ifndef _POLIT_H
#define _POLIT_H

#include "EquipType.h"

class StarSystem;

namespace Polit {

	enum Alignment {
		POL_INVALID,
		POL_NONE,
		POL_EARTH,
		POL_CONFED,
		POL_MAX
	};
	Polit::Alignment GetAlignmentForStarSystem(StarSystem *s, fixed human_infestedness);
	bool IsCommodityLegal(StarSystem *s, Equip::Type t);
	extern const char * const desc[];
}

#endif /* _POLIT_H */
