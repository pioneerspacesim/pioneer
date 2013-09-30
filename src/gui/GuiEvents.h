// Copyright © 2008-2013 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#ifndef _GUIEVENTS_H
#define _GUIEVENTS_H

namespace Gui {
	struct MouseButtonEvent {
		Uint8 isdown;
		Uint8 button;
		float x, y; // widget coords
		float screenX, screenY; // screen coords
		static const Uint8 BUTTON_WHEELUP   = 0xfe;
		static const Uint8 BUTTON_WHEELDOWN = 0xff;
	};
	struct MouseMotionEvent {
		float x, y; // widget coords
		float screenX, screenY; // screen coords
	};
}

#endif /* _GUIEVENTS_H */
