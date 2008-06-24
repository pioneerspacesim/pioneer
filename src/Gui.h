#ifndef _GUI_H
#define _GUI_H

#include "libs.h"

namespace Gui {

	namespace Color {
		extern const float bg[];
		extern const float bgShadow[];
	}

	void HandleSDLEvent(SDL_Event *event);
	void Draw();
	void Init(int screen_width, int screen_height, int ui_width, int ui_height);
}

#include "GuiEvents.h"

namespace Gui {
	namespace RawEvents {
		extern sigc::signal<void, SDL_MouseButtonEvent *> onMouseDown;
		extern sigc::signal<void, SDL_MouseButtonEvent *> onMouseUp;
		extern sigc::signal<void, SDL_KeyboardEvent *> onKeyDown;
		extern sigc::signal<void, SDL_KeyboardEvent *> onKeyUp;
	}
}

#include "GuiWidget.h"
#include "GuiImage.h"
#include "GuiButton.h"
#include "GuiToggleButton.h"
#include "GuiMultiStateImageButton.h"
#include "GuiImageButton.h"
#include "GuiISelectable.h"
#include "GuiRadioButton.h"
#include "GuiImageRadioButton.h"
#include "GuiRadioGroup.h"
#include "GuiFixed.h"
#include "GuiLabel.h"
#include "GuiScreen.h"

#endif /* _GUI_H */
