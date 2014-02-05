// Copyright © 2008-2014 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#ifndef _GUI_H
#define _GUI_H

#include "libs.h"
#include "Color.h"

namespace Graphics {
	class Renderer;
	class WindowSDL;
	class RenderState;
}

namespace Gui {

	namespace Theme {
		void DrawRect(const vector2f&, const vector2f &size, const Color&, Graphics::RenderState*);
		void DrawRoundEdgedRect(const float size[2], float rad, const Color&, Graphics::RenderState*);
		void DrawIndent(const float size[2], Graphics::RenderState*);
		void DrawOutdent(const float size[2], Graphics::RenderState*);
		void DrawHollowRect(const float size[2], const Color&, Graphics::RenderState*);
		namespace Colors {
			extern const Color bg;
			extern const Color bgShadow;
			extern const Color tableHeading;
		}
	}


	void HandleSDLEvent(SDL_Event *event);
	void Draw();
	sigc::connection AddTimer(Uint32 ms, sigc::slot<void> slot);
	void Init(Graphics::Renderer *renderer, int screen_width, int screen_height, int ui_width, int ui_height);
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
#include "GuiTexturedQuad.h"

#endif /* _GUI_H */
