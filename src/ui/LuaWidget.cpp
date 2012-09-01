#include "Widget.h"
#include "LuaObject.h"
#include "LuaConstants.h"
#include "LuaSignal.h"

namespace UI {

class LuaWidget {
public:

	static int l_set_font_size(lua_State *l) {
		UI::Widget *w = LuaObject<UI::Widget>::CheckFromLua(1);
		UI::Widget::FontSize fontSize = static_cast<UI::Widget::FontSize>(LuaConstants::GetConstantFromArg(l, "UIFontSize", 2));
		w->SetFontSize(fontSize);
		lua_pushvalue(l, 1);
		return 1;
	}


	static int l_attr_on_key_down(lua_State *l) {
		UI::Widget *w = LuaObject<UI::Widget>::CheckFromLua(1);
		LuaSignal<const KeyboardEvent &>().Wrap(l, w->onKeyDown);
		return 1;
	}

	static int l_attr_on_key_up(lua_State *l) {
		UI::Widget *w = LuaObject<UI::Widget>::CheckFromLua(1);
		LuaSignal<const KeyboardEvent &>().Wrap(l, w->onKeyUp);
		return 1;
	}

	static int l_attr_on_key_press(lua_State *l) {
		UI::Widget *w = LuaObject<UI::Widget>::CheckFromLua(1);
		LuaSignal<const KeyboardEvent &>().Wrap(l, w->onKeyPress);
		return 1;
	}

	static int l_attr_on_mouse_down(lua_State *l) {
		UI::Widget *w = LuaObject<UI::Widget>::CheckFromLua(1);
		LuaSignal<const MouseButtonEvent &>().Wrap(l, w->onMouseDown);
		return 1;
	}

	static int l_attr_on_mouse_up(lua_State *l) {
		UI::Widget *w = LuaObject<UI::Widget>::CheckFromLua(1);
		LuaSignal<const MouseButtonEvent &>().Wrap(l, w->onMouseUp);
		return 1;
	}

	static int l_attr_on_mouse_move(lua_State *l) {
		UI::Widget *w = LuaObject<UI::Widget>::CheckFromLua(1);
		LuaSignal<const MouseMotionEvent &>().Wrap(l, w->onMouseMove);
		return 1;
	}

	static int l_attr_on_mouse_wheel(lua_State *l) {
		UI::Widget *w = LuaObject<UI::Widget>::CheckFromLua(1);
		LuaSignal<const MouseWheelEvent &>().Wrap(l, w->onMouseWheel);
		return 1;
	}

	static int l_attr_on_mouse_over(lua_State *l) {
		UI::Widget *w = LuaObject<UI::Widget>::CheckFromLua(1);
		LuaSignal<>().Wrap(l, w->onMouseOver);
		return 1;
	}

	static int l_attr_on_mouse_out(lua_State *l) {
		UI::Widget *w = LuaObject<UI::Widget>::CheckFromLua(1);
		LuaSignal<>().Wrap(l, w->onMouseOut);
		return 1;
	}

	static int l_attr_on_click(lua_State *l) {
		UI::Widget *w = LuaObject<UI::Widget>::CheckFromLua(1);
		LuaSignal<>().Wrap(l, w->onClick);
		return 1;
	}

};

}

using namespace UI;

template <> const char *LuaObject<UI::Widget>::s_type = "UI.Widget";

template <> void LuaObject<UI::Widget>::RegisterClass()
{
	static const luaL_Reg l_methods[] = {
		{ "SetFontSize", LuaWidget::l_set_font_size },
		{ 0, 0 }
	};

	static const luaL_Reg l_attrs[] = {
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
