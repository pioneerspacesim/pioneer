#include "libs.h"
#include "Polit.h"
#include "StarSystem.h"
#include "Sector.h"
#include "custom_starsystems.h"
#include "EquipType.h"

namespace Polit {

const char * const desc[POL_MAX] = {
	"<invalid turd>",
	"No central governance.",
	"Member of the Earth Federation.",
	"Member of the Confederation of Independent Systems.",
};

Polit::Alignment GetAlignmentForStarSystem(StarSystem *s, fixed human_infestedness)
{
	int sx, sy, sys_idx;
	s->GetPos(&sx, &sy, &sys_idx);

	Sector sec(sx, sy);
	
	/* from custom system definition */
	if (sec.m_systems[sys_idx].customSys) {
		Polit::Alignment t = sec.m_systems[sys_idx].customSys->polit;
		if (t != POL_INVALID) return t;
	}

	const unsigned long _init[3] = { sx, sy, sys_idx };
	MTRand rand(_init, 3);

	if ((sx == 0) && (sy == 0) && (sys_idx == 0)) {
		return Polit::POL_EARTH;
	} else if (human_infestedness > 0) {
		return static_cast<Alignment>(rand.Int32(POL_EARTH, POL_MAX-1));
	} else {
		return POL_NONE;
	}
}

bool IsCommodityLegal(StarSystem *s, Equip::Type t)
{
	int sx, sy, sys_idx;
	s->GetPos(&sx, &sy, &sys_idx);
	const unsigned long _init[3] = { sx, sy, sys_idx };
	MTRand rand(_init, 3);

	Polit::Alignment a = s->GetPoliticalType();

	if (a == POL_NONE) return true;

	switch (t) {
		case Equip::ANIMAL_MEAT:
			if ((a == POL_EARTH) || (a == POL_CONFED)) return rand.Int32(4)!=0;
			else return true;
		case Equip::LIQUOR:
			if ((a != POL_EARTH) && (a != POL_CONFED)) return rand.Int32(8)!=0;
			else return true;
		case Equip::HAND_WEAPONS:
			if (a == POL_EARTH) return false;
			if (a == POL_CONFED) return rand.Int32(3)!=0;
			else return rand.Int32(2) == 0;
		case Equip::BATTLE_WEAPONS:
			if ((a != POL_EARTH) && (a != POL_CONFED)) return rand.Int32(3)==0;
			return false;
		case Equip::NERVE_GAS:
			if ((a != POL_EARTH) && (a != POL_CONFED)) return rand.Int32(10)==0;
			return false;
		case Equip::NARCOTICS:
			if (a == POL_EARTH) return false;
			if (a == POL_CONFED) return rand.Int32(7)==0;
			else return rand.Int32(2)==0;
		default: return true;
	}
}

}

