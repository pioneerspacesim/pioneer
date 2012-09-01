#include "List.h"
#include "LuaObject.h"

namespace UI {

class LuaList {
public:

	static int l_add_option(lua_State *l) {
		UI::List *list = LuaObject<UI::List>::CheckFromLua(1);
		list->AddOption(luaL_checkstring(l, 2));
		lua_pushvalue(l, 1);
		return 1;
	}

};

}

using namespace UI;

template <> const char *LuaObject<UI::List>::s_type = "UI.List";

template <> void LuaObject<UI::List>::RegisterClass()
{
	static const char *l_parent = "UI.Container";

	static const luaL_Reg l_methods[] = {
		{ "AddOption", LuaList::l_add_option },
        { 0, 0 }
	};

	LuaObjectBase::CreateClass(s_type, l_parent, l_methods, 0, 0);
	LuaObjectBase::RegisterPromotion(l_parent, s_type, LuaObject<UI::List>::DynamicCastPromotionTest);
}
