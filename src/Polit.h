#ifndef _POLIT_H
#define _POLIT_H

class StarSystem;

namespace Polit {
	enum Type {
		POL_INVALID,
		POL_NONE,
		POL_EARTH,
		POL_CONFED,
		POL_MAX
	};
	Polit::Type GetTypeForStarSystem(StarSystem *s, int human_infestedness);
	extern const char * const desc[];
}

#endif /* _POLIT_H */
