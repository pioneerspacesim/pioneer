#include "Colors.h"

namespace Colors {
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