#include "Context.h"
#include "LuaObject.h"

namespace UI {

class LuaContext {
public:
	static int l_hbox(lua_State *l) {
		UI::Context *c = LuaObject<UI::Context>::CheckFromLua(1);
		if (lua_gettop(l) > 1)
			LuaObject<UI::HBox>::PushToLua(c->HBox(luaL_checknumber(l, 2)));
		else
			LuaObject<UI::HBox>::PushToLua(c->HBox());
		return 1;
	}
	
	static int l_vbox(lua_State *l) {
		UI::Context *c = LuaObject<UI::Context>::CheckFromLua(1);
		if (lua_gettop(l) > 1)
			LuaObject<UI::VBox>::PushToLua(c->VBox(luaL_checknumber(l, 2)));
		else
			LuaObject<UI::VBox>::PushToLua(c->VBox());
		return 1;
	}
	
	static int l_grid(lua_State *l) {
		return 0;
	}
	
	static int l_background(lua_State *l) {
		UI::Context *c = LuaObject<UI::Context>::CheckFromLua(1);
		LuaObject<UI::Background>::PushToLua(c->Background());
		return 1;
	}
	
	static int l_colorbackground(lua_State *l) {
		UI::Context *c = LuaObject<UI::Context>::CheckFromLua(1);
		float r = luaL_checknumber(l, 2);
		float g = luaL_checknumber(l, 3);
		float b = luaL_checknumber(l, 4);
		float a = 1.0f;
		if (lua_gettop(l) > 4)
			a = luaL_checknumber(l, 5);
		LuaObject<UI::ColorBackground>::PushToLua(c->ColorBackground(Color(r,g,b,a)));
		return 1;
	}
	
	static int l_margin(lua_State *l) {
		UI::Context *c = LuaObject<UI::Context>::CheckFromLua(1);
		LuaObject<UI::Margin>::PushToLua(c->Margin(luaL_checknumber(l, 2)));
		return 1;
	}
	
	static int l_align(lua_State *l) {
		return 0;
	}
	
	static int l_scroller(lua_State *l) {
		UI::Context *c = LuaObject<UI::Context>::CheckFromLua(1);
		LuaObject<UI::Scroller>::PushToLua(c->Scroller());
		return 1;
	}
	
	static int l_image(lua_State *l) {
		return 0;
	}
	
	static int l_label(lua_State *l) {
		UI::Context *c = LuaObject<UI::Context>::CheckFromLua(1);
		LuaObject<UI::Label>::PushToLua(c->Label(luaL_checkstring(l, 2)));
		return 1;
	}
	
	static int l_multilinetext(lua_State *l) {
		UI::Context *c = LuaObject<UI::Context>::CheckFromLua(1);
		LuaObject<UI::MultiLineText>::PushToLua(c->MultiLineText(luaL_checkstring(l, 2)));
		return 1;
	}
	
	static int l_button(lua_State *l) {
		UI::Context *c = LuaObject<UI::Context>::CheckFromLua(1);
		LuaObject<UI::Button>::PushToLua(c->Button());
		return 1;
	}
	
	static int l_checkbox(lua_State *l) {
		UI::Context *c = LuaObject<UI::Context>::CheckFromLua(1);
		LuaObject<UI::CheckBox>::PushToLua(c->CheckBox());
		return 1;
	}
	
	static int l_hslider(lua_State *l) {
		UI::Context *c = LuaObject<UI::Context>::CheckFromLua(1);
		LuaObject<UI::HSlider>::PushToLua(c->HSlider());
		return 1;
	}
	
	static int l_vslider(lua_State *l) {
		UI::Context *c = LuaObject<UI::Context>::CheckFromLua(1);
		LuaObject<UI::VSlider>::PushToLua(c->VSlider());
		return 1;
	}
	
	static int l_list(lua_State *l) {
		UI::Context *c = LuaObject<UI::Context>::CheckFromLua(1);
		LuaObject<UI::List>::PushToLua(c->List());
		return 1;
	}
	
	static int l_dropdown(lua_State *l) {
		UI::Context *c = LuaObject<UI::Context>::CheckFromLua(1);
		LuaObject<UI::DropDown>::PushToLua(c->DropDown());
		return 1;
	}
};

}

using namespace UI;

template <> const char *LuaObject<UI::Context>::s_type = "UI.Context";

template <> void LuaObject<UI::Context>::RegisterClass()
{
	static const char *l_parent = "UI.Single";

	static const luaL_Reg l_methods[] = {
		{ "HBox",            LuaContext::l_hbox            },
		{ "VBox",            LuaContext::l_vbox            },
		{ "Grid",            LuaContext::l_grid            },
		{ "Background",      LuaContext::l_background      },
		{ "ColorBackground", LuaContext::l_colorbackground },
		{ "Margin",          LuaContext::l_margin          },
		{ "Align",           LuaContext::l_align           },
		{ "Scroller",        LuaContext::l_scroller        },
		{ "Image",           LuaContext::l_image           },
		{ "Label",           LuaContext::l_label           },
		{ "MultiLineText",   LuaContext::l_multilinetext   },
		{ "Button",          LuaContext::l_button          },
		{ "CheckBox",        LuaContext::l_checkbox        },
		{ "HSlider",         LuaContext::l_hslider         },
		{ "VSlider",         LuaContext::l_vslider         },
		{ "List",            LuaContext::l_list            },
		{ "DropDown",        LuaContext::l_dropdown        },

        { 0, 0 }
	};

	LuaObjectBase::CreateClass(s_type, l_parent, l_methods, 0, 0);
	LuaObjectBase::RegisterPromotion(l_parent, s_type, LuaObject<UI::Context>::DynamicCastPromotionTest);
}
