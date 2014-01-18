#ifndef HUDCOLORS_H
#define HUDCOLORS_H

/*
 * The place for colors and possibly bitmaps.
 * XXX move to Lua or skins
 */

#include "Color.h"

namespace HudColors {
	extern const Color HUD_MESSAGE;
	extern const Color HUD_TARGET_INFO;
	extern const Color HUD_TEXT;

	//iff colors
	extern const Color IFF_UNKNOWN;
	extern const Color IFF_NEUTRAL;
	extern const Color IFF_ALLY;
	extern const Color IFF_HOSTILE;
}

#endif
