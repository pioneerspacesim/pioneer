#ifndef _GUIEVENTS_H
#define _GUIEVENTS_H

namespace Gui {
	struct MouseButtonEvent {
		Uint8 isdown;
		Uint8 button;
		float x, y;
	};
}

#endif /* _GUIEVENTS_H */
