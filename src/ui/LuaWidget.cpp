#include "Widget.h"
#include "LuaObject.h"
#include "LuaConstants.h"

namespace UI {

template <typename T0 = sigc::nil, typename T1 = sigc::nil>
class LuaSignal {
public:

	typedef typename sigc::signal<bool,T0,T1>::template accumulated<UI::Widget::EventHandlerResultAccumulator> signal_type;

	static bool trampoline(lua_State *l, int idx);

	static int l_connect(lua_State *l) {
		luaL_checktype(l, 1, LUA_TTABLE);
		luaL_checktype(l, 2, LUA_TFUNCTION);

		lua_pushstring(l, "__signal");
		lua_rawget(l, 1);
		if (!lua_islightuserdata(l, -1)) {
			luaL_error(l, "signal pointer not found");
			return 0;
		}
		signal_type *signal = static_cast<signal_type*>(lua_touserdata(l, -1));
		lua_pop(l, 1);

		lua_getfield(l, LUA_REGISTRYINDEX, "PiUISignal");
		if (lua_isnil(l, -1)) {
			lua_newtable(l);
			lua_pushvalue(l, -1);
			lua_setfield(l, LUA_REGISTRYINDEX, "PiUISignal");
		}

		int idx = lua_rawlen(l, -1)+1;
		lua_pushvalue(l, 2);
		lua_rawseti(l, -2, idx);

		signal->connect(sigc::bind(sigc::ptr_fun(&trampoline), l, idx));

		return 0;
	}

	void Wrap(lua_State *l, signal_type &signal) {
		lua_newtable(l);

		lua_pushstring(l, "Connect");
		lua_pushcfunction(l, l_connect);
		lua_rawset(l, -3);

		lua_pushstring(l, "__signal");
		lua_pushlightuserdata(l, &signal);
		lua_rawset(l, -3);
	}
};

template <> bool LuaSignal<>::trampoline(lua_State *l, int idx) {
	LUA_DEBUG_START(l);

	lua_getfield(l, LUA_REGISTRYINDEX, "PiUISignal");
	assert(lua_istable(l, -1));

	lua_rawgeti(l, -1, idx);
	assert(lua_isfunction(l, -1));

	pi_lua_protected_call(l, 0, 1);
	bool ret = lua_toboolean(l, -1);

	lua_pop(l, 2);

	LUA_DEBUG_END(l, 0);

	return ret;
}


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
		return 0;
	}

	static int l_attr_on_key_up(lua_State *l) {
		UI::Widget *w = LuaObject<UI::Widget>::CheckFromLua(1);
		return 0;
	}

	static int l_attr_on_key_press(lua_State *l) {
		UI::Widget *w = LuaObject<UI::Widget>::CheckFromLua(1);
		return 0;
	}

	static int l_attr_on_mouse_down(lua_State *l) {
		UI::Widget *w = LuaObject<UI::Widget>::CheckFromLua(1);
		return 0;
	}

	static int l_attr_on_mouse_up(lua_State *l) {
		UI::Widget *w = LuaObject<UI::Widget>::CheckFromLua(1);
		return 0;
	}

	static int l_attr_on_mouse_move(lua_State *l) {
		UI::Widget *w = LuaObject<UI::Widget>::CheckFromLua(1);
		return 0;
	}

	static int l_attr_on_mouse_wheel(lua_State *l) {
		UI::Widget *w = LuaObject<UI::Widget>::CheckFromLua(1);
		return 0;
	}

	static int l_attr_on_mouse_over(lua_State *l) {
		UI::Widget *w = LuaObject<UI::Widget>::CheckFromLua(1);
		return 0;
	}

	static int l_attr_on_mouse_out(lua_State *l) {
		UI::Widget *w = LuaObject<UI::Widget>::CheckFromLua(1);
		return 0;
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
