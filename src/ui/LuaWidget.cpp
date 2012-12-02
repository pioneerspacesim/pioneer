// Copyright © 2008-2012 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#include "Widget.h"
#include "LuaObject.h"
#include "LuaConstants.h"
#include "LuaSignal.h"

namespace UI {

class LuaWidget {
public:

	static int l_set_font_size(lua_State *l) {
		UI::Widget *w = LuaObject<UI::Widget>::CheckFromLua(1);
		UI::Widget::Font font = static_cast<UI::Widget::Font>(LuaConstants::GetConstantFromArg(l, "UIFont", 2));
		w->SetFont(font);
		lua_pushvalue(l, 1);
		return 1;
	}


	static int l_disable(lua_State *l) {
		UI::Widget *w = LuaObject<UI::Widget>::CheckFromLua(1);
		w->Disable();
		return 0;
	}

	static int l_enable(lua_State *l) {
		UI::Widget *w = LuaObject<UI::Widget>::CheckFromLua(1);
		w->Disable();
		return 0;
	}

	static int l_attr_disabled(lua_State *l) {
		UI::Widget *w = LuaObject<UI::Widget>::CheckFromLua(1);
		lua_pushboolean(l, w->IsDisabled());
		return 1;
	}


	static int l_attr_on_key_down(lua_State *l) {
		UI::Widget *w = LuaObject<UI::Widget>::CheckFromLua(1);
		LuaSignalAccumulated<const KeyboardEvent &>().Wrap(l, w->onKeyDown);
		return 1;
	}

	static int l_attr_on_key_up(lua_State *l) {
		UI::Widget *w = LuaObject<UI::Widget>::CheckFromLua(1);
		LuaSignalAccumulated<const KeyboardEvent &>().Wrap(l, w->onKeyUp);
		return 1;
	}

	static int l_attr_on_key_press(lua_State *l) {
		UI::Widget *w = LuaObject<UI::Widget>::CheckFromLua(1);
		LuaSignalAccumulated<const KeyboardEvent &>().Wrap(l, w->onKeyPress);
		return 1;
	}

	static int l_attr_on_mouse_down(lua_State *l) {
		UI::Widget *w = LuaObject<UI::Widget>::CheckFromLua(1);
		LuaSignalAccumulated<const MouseButtonEvent &>().Wrap(l, w->onMouseDown);
		return 1;
	}

	static int l_attr_on_mouse_up(lua_State *l) {
		UI::Widget *w = LuaObject<UI::Widget>::CheckFromLua(1);
		LuaSignalAccumulated<const MouseButtonEvent &>().Wrap(l, w->onMouseUp);
		return 1;
	}

	static int l_attr_on_mouse_move(lua_State *l) {
		UI::Widget *w = LuaObject<UI::Widget>::CheckFromLua(1);
		LuaSignalAccumulated<const MouseMotionEvent &>().Wrap(l, w->onMouseMove);
		return 1;
	}

	static int l_attr_on_mouse_wheel(lua_State *l) {
		UI::Widget *w = LuaObject<UI::Widget>::CheckFromLua(1);
		LuaSignalAccumulated<const MouseWheelEvent &>().Wrap(l, w->onMouseWheel);
		return 1;
	}

	static int l_attr_on_mouse_over(lua_State *l) {
		UI::Widget *w = LuaObject<UI::Widget>::CheckFromLua(1);
		LuaSignalAccumulated<>().Wrap(l, w->onMouseOver);
		return 1;
	}

	static int l_attr_on_mouse_out(lua_State *l) {
		UI::Widget *w = LuaObject<UI::Widget>::CheckFromLua(1);
		LuaSignalAccumulated<>().Wrap(l, w->onMouseOut);
		return 1;
	}

	static int l_attr_on_click(lua_State *l) {
		UI::Widget *w = LuaObject<UI::Widget>::CheckFromLua(1);
		LuaSignalAccumulated<>().Wrap(l, w->onClick);
		return 1;
	}

};

}

using namespace UI;

template <> const char *LuaObject<UI::Widget>::s_type = "UI.Widget";

template <> void LuaObject<UI::Widget>::RegisterClass()
{
	static const luaL_Reg l_methods[] = {
		{ "SetFont", LuaWidget::l_set_font_size },

		{ "Disable", LuaWidget::l_disable       },
		{ "Enable",  LuaWidget::l_enable        },

		{ 0, 0 }
	};

	static const luaL_Reg l_attrs[] = {
		{ "disabled",     LuaWidget::l_attr_disabled       },

		{ "onKeyDown",    LuaWidget::l_attr_on_key_down    },
		{ "onKeyUp",      LuaWidget::l_attr_on_key_up      },
		{ "onKeyPress",   LuaWidget::l_attr_on_key_press   },
		{ "onMouseDown",  LuaWidget::l_attr_on_mouse_down  },
		{ "onMouseUp",    LuaWidget::l_attr_on_mouse_up    },
		{ "onMouseMove",  LuaWidget::l_attr_on_mouse_move  },
		{ "onMouseWheel", LuaWidget::l_attr_on_mouse_wheel },
		{ "onMouseOver",  LuaWidget::l_attr_on_mouse_over  },
		{ "onMouseOut",   LuaWidget::l_attr_on_mouse_out   },
		{ "onClick",      LuaWidget::l_attr_on_click       },

		{ 0, 0 }
	};

	LuaObjectBase::CreateClass(s_type, 0, l_methods, l_attrs, 0);
}
