// Copyright © 2008-2014 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#include "HudColors.h"

namespace HudColors {
	const Color HUD_MESSAGE     = Color::WHITE;
	const Color HUD_TARGET_INFO = Color::WHITE;
	const Color HUD_TEXT        = Color(0.0f,1.0f,0.0f,0.8f);

	//iff colors
	//these are too intense
	extern const Color IFF_UNKNOWN = Color::GRAY;
	extern const Color IFF_NEUTRAL = Color::BLUE;
	extern const Color IFF_ALLY    = Color::GREEN;
	extern const Color IFF_HOSTILE = Color::RED;
}
