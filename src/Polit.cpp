#include "libs.h"
#include "Polit.h"
#include "StarSystem.h"
#include "Sector.h"
#include "custom_starsystems.h"

namespace Polit {

const char * const desc[POL_MAX] = {
	"<invalid turd>",
	"No central governance.",
	"Member of the Earth Federation.",
	"Member of the Confederation of Independent Systems.",
};

Polit::Type GetTypeForStarSystem(StarSystem *s, fixed human_infestedness)
{
	int sx, sy, sys_idx;
	s->GetPos(&sx, &sy, &sys_idx);

	Sector sec(sx, sy);
	
	/* from custom system definition */
	if (sec.m_systems[sys_idx].customSys) {
		Polit::Type t = sec.m_systems[sys_idx].customSys->polit;
		if (t != POL_INVALID) return t;
	}

	const unsigned long _init[3] = { sx, sy, sys_idx };
	MTRand rand(_init, 3);

	if ((sx == 0) && (sy == 0) && (sys_idx == 0)) {
		return Polit::POL_EARTH;
	} else if (human_infestedness > 0) {
		return static_cast<Type>(rand.Int32(POL_EARTH, POL_MAX-1));
	} else {
		return POL_NONE;
	}
}

}

