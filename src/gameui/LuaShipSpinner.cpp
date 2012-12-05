// Copyright © 2008-2012 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#include "ShipSpinner.h"
#include "LuaObject.h"

namespace GameUI {

class LuaShipSpinner {
public:

	static int l_new(lua_State *l) {
		UI::Context *c = LuaObject<UI::Context>::CheckFromLua(1);
		ShipFlavour f = ShipFlavour::FromLuaTable(l, 2);
		LuaObject<ShipSpinner>::PushToLua(new ShipSpinner(c, f));
		return 1;
	}

};

}

using namespace GameUI;

template <> const char *LuaObject<GameUI::ShipSpinner>::s_type = "UI.Game.ShipSpinner";

template <> void LuaObject<GameUI::ShipSpinner>::RegisterClass()
{
	static const char *l_parent = "UI.Widget";

	static const luaL_Reg l_methods[] = {
		{ "New", LuaShipSpinner::l_new },
        { 0, 0 }
	};

	LuaObjectBase::CreateClass(s_type, l_parent, l_methods, 0, 0);
	LuaObjectBase::RegisterPromotion(l_parent, s_type, LuaObject<GameUI::ShipSpinner>::DynamicCastPromotionTest);
}
