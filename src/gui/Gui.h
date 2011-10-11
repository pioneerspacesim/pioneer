#ifndef _GUI_H
#define _GUI_H

#include "libs.h"
#include "Color.h"

namespace Gui {

	namespace Theme {
		void DrawRoundEdgedRect(const float size[2], float rad);
		void DrawIndent(const float size[2]);
		void DrawOutdent(const float size[2]);
		void DrawHollowRect(const float size[2]);
		namespace Colors {
			extern const float bg[];
			extern const float bgShadow[];
			extern const float tableHeading[];
		}
	}


	void HandleSDLEvent(SDL_Event *event);
	void Draw();
	void MainLoopIteration();
	sigc::connection AddTimer(Uint32 ms, sigc::slot<void> slot);
	void Init(int screen_width, int screen_height, int ui_width, int ui_height);
	void Uninit();
}

#include "GuiEvents.h"

namespace Gui {
	namespace RawEvents {
		extern sigc::signal<void, MouseMotionEvent *> onMouseMotion;
		extern sigc::signal<void, MouseButtonEvent *> onMouseDown;
		extern sigc::signal<void, MouseButtonEvent *> onMouseUp;
		extern sigc::signal<void, SDL_KeyboardEvent *> onKeyDown;
		extern sigc::signal<void, SDL_KeyboardEvent *> onKeyUp;
		extern sigc::signal<void, SDL_JoyAxisEvent *> onJoyAxisMotion;
		extern sigc::signal<void, SDL_JoyButtonEvent *> onJoyButtonDown;
		extern sigc::signal<void, SDL_JoyButtonEvent *> onJoyButtonUp;
		extern sigc::signal<void, SDL_JoyHatEvent *> onJoyHatMotion;
	}
}

#include "GuiWidget.h"
#include "GuiAdjustment.h"
#include "GuiImage.h"
#include "GuiButton.h"
#include "GuiRepeaterButton.h"
#include "GuiToggleButton.h"
#include "GuiMultiStateImageButton.h"
#include "GuiImageButton.h"
#include "GuiISelectable.h"
#include "GuiRadioButton.h"
#include "GuiImageRadioButton.h"
#include "GuiRadioGroup.h"
#include "GuiBox.h"
#include "GuiFixed.h"
#include "GuiVScrollPortal.h"
#include "GuiVScrollBar.h"
#include "GuiTextLayout.h"
#include "GuiLabel.h"
#include "GuiToolTip.h"
#include "GuiTabbed.h"
#include "GuiTextEntry.h"
#include "GuiMeterBar.h"
#include "GuiLabelSet.h"
#include "GuiScreen.h"
#include "GuiStack.h"
#include "GuiGradient.h"

#endif /* _GUI_H */
