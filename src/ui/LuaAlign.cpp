// Copyright © 2008-2014 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#include "Align.h"
#include "LuaObject.h"

namespace UI {

class LuaAlign {
public:

};

}

using namespace UI;

template <> const char *LuaObject<UI::Align>::s_type = "UI.Align";

template <> void LuaObject<UI::Align>::RegisterClass()
{
	static const char *l_parent = "UI.Single";

	static const luaL_Reg l_methods[] = {

		{ 0, 0 }
	};

	LuaObjectBase::CreateClass(s_type, l_parent, l_methods, 0, 0);
	LuaObjectBase::RegisterPromotion(l_parent, s_type, LuaObject<UI::Align>::DynamicCastPromotionTest);
}
