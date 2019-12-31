// Copyright © 2008-2020 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#ifndef _GUI_H
#define _GUI_H

#include "Color.h"
#include <sigc++/sigc++.h>
#include <SDL.h>

namespace Graphics {
	class Renderer;
	class VertexBuffer;
	class RenderState;
} // namespace Graphics

namespace Gui {

	namespace Theme {
		void DrawIndent(const float size[2], Graphics::RenderState *);
		void DrawOutdent(const float size[2], Graphics::RenderState *);
		void DrawHollowRect(const float size[2], const Color &, Graphics::RenderState *);
		namespace Colors {
			extern const Color bg;
			extern const Color bgShadow;
			extern const Color tableHeading;
		} // namespace Colors
	} // namespace Theme

	void HandleSDLEvent(SDL_Event *event);
	void Draw();
	sigc::connection AddTimer(Uint32 ms, sigc::slot<void> slot);
	void Init(Graphics::Renderer *renderer, int screen_width, int screen_height, int ui_width, int ui_height);
	void Uninit();
} // namespace Gui

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
	} // namespace RawEvents
} // namespace Gui

#include "GuiAdjustment.h"
#include "GuiBox.h"
#include "GuiButton.h"
#include "GuiFixed.h"
#include "GuiISelectable.h"
#include "GuiImage.h"
#include "GuiImageButton.h"
#include "GuiImageRadioButton.h"
#include "GuiLabel.h"
#include "GuiLabelSet.h"
#include "GuiMeterBar.h"
#include "GuiMultiStateImageButton.h"
#include "GuiRadioButton.h"
#include "GuiRadioGroup.h"
#include "GuiScreen.h"
#include "GuiStack.h"
#include "GuiTabbed.h"
#include "GuiTextEntry.h"
#include "GuiTextLayout.h"
#include "GuiTexturedQuad.h"
#include "GuiToggleButton.h"
#include "GuiToolTip.h"
#include "GuiVScrollBar.h"
#include "GuiVScrollPortal.h"
#include "GuiWidget.h"

#endif /* _GUI_H */
