// Copyright © 2008-2013 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#include "Gauge.h"
#include "LuaObject.h"

namespace UI {

class LuaGauge {
public:

};

}

using namespace UI;

template <> const char *LuaObject<UI::Gauge>::s_type = "UI.Gauge";

template <> void LuaObject<UI::Gauge>::RegisterClass()
{
	static const char *l_parent = "UI.Widget";

	static const luaL_Reg l_methods[] = {

		{ 0, 0 }
	};

	LuaObjectBase::CreateClass(s_type, l_parent, l_methods, 0, 0);
	LuaObjectBase::RegisterPromotion(l_parent, s_type, LuaObject<UI::Gauge>::DynamicCastPromotionTest);
}
