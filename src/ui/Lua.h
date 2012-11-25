// Copyright © 2008-2012 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#ifndef UI_LUA_H
#define UI_LUA_H

#include "LuaObject.h"
#include "Context.h"

namespace UI {
namespace Lua {

	void Init();

	// get widget from stack. handles table.widget format as well
	UI::Widget *GetWidget(lua_State *l, int idx);
	UI::Widget *CheckWidget(lua_State *l, int idx);

}
}

template <> class LuaAcquirer<UI::Align> : public LuaAcquirerRefCounted {};
template <> class LuaAcquirer<UI::Background> : public LuaAcquirerRefCounted {};
template <> class LuaAcquirer<UI::Box> : public LuaAcquirerRefCounted {};
template <> class LuaAcquirer<UI::HBox> : public LuaAcquirerRefCounted {};
template <> class LuaAcquirer<UI::VBox> : public LuaAcquirerRefCounted {};
template <> class LuaAcquirer<UI::Button> : public LuaAcquirerRefCounted {};
template <> class LuaAcquirer<UI::CheckBox> : public LuaAcquirerRefCounted {};
template <> class LuaAcquirer<UI::ColorBackground> : public LuaAcquirerRefCounted {};
template <> class LuaAcquirer<UI::Container> : public LuaAcquirerRefCounted {};
template <> class LuaAcquirer<UI::Context> : public LuaAcquirerRefCounted {};
template <> class LuaAcquirer<UI::DropDown> : public LuaAcquirerRefCounted {};
template <> class LuaAcquirer<UI::Grid> : public LuaAcquirerRefCounted {};
template <> class LuaAcquirer<UI::Icon> : public LuaAcquirerRefCounted {};
template <> class LuaAcquirer<UI::Image> : public LuaAcquirerRefCounted {};
template <> class LuaAcquirer<UI::Label> : public LuaAcquirerRefCounted {};
template <> class LuaAcquirer<UI::List> : public LuaAcquirerRefCounted {};
template <> class LuaAcquirer<UI::Margin> : public LuaAcquirerRefCounted {};
template <> class LuaAcquirer<UI::MultiLineText> : public LuaAcquirerRefCounted {};
template <> class LuaAcquirer<UI::Scroller> : public LuaAcquirerRefCounted {};
template <> class LuaAcquirer<UI::Single> : public LuaAcquirerRefCounted {};
template <> class LuaAcquirer<UI::Slider> : public LuaAcquirerRefCounted {};
template <> class LuaAcquirer<UI::SmallButton> : public LuaAcquirerRefCounted {};
template <> class LuaAcquirer<UI::TextEntry> : public LuaAcquirerRefCounted {};
template <> class LuaAcquirer<UI::HSlider> : public LuaAcquirerRefCounted {};
template <> class LuaAcquirer<UI::VSlider> : public LuaAcquirerRefCounted {};
template <> class LuaAcquirer<UI::Widget> : public LuaAcquirerRefCounted {};

#endif
