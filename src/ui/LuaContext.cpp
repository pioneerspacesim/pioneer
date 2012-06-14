#include "Context.h"
#include "LuaObject.h"

namespace UI {

class LuaContext {
public:
	static int l_hbox(lua_State *l) {
		return 0;
	}
	
	static int l_vbox(lua_State *l) {
		return 0;
	}
	
	static int l_grid(lua_State *l) {
		return 0;
	}
	
	static int l_background(lua_State *l) {
		return 0;
	}
	
	static int l_colorbackground(lua_State *l) {
		return 0;
	}
	
	static int l_margin(lua_State *l) {
		return 0;
	}
	
	static int l_align(lua_State *l) {
		return 0;
	}
	
	static int l_scroller(lua_State *l) {
		return 0;
	}
	
	static int l_image(lua_State *l) {
		return 0;
	}
	
	static int l_label(lua_State *l) {
		return 0;
	}
	
	static int l_multilinetext(lua_State *l) {
		return 0;
	}
	
	static int l_button(lua_State *l) {
		return 0;
	}
	
	static int l_checkbox(lua_State *l) {
		return 0;
	}
	
	static int l_hslider(lua_State *l) {
		return 0;
	}
	
	static int l_vslider(lua_State *l) {
		return 0;
	}
	
	static int l_list(lua_State *l) {
		return 0;
	}
	
	static int l_dropdown(lua_State *l) {
		return 0;
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
