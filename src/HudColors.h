// Copyright © 2008-2014 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

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
