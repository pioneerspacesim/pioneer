// Copyright © 2008-2014 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#include "Image.h"
#include "LuaObject.h"

namespace UI {

class LuaImage {
public:

};

}

using namespace UI;

template <> const char *LuaObject<UI::Image>::s_type = "UI.Image";

template <> void LuaObject<UI::Image>::RegisterClass()
{
	static const char *l_parent = "UI.Widget";

	static const luaL_Reg l_methods[] = {

		{ 0, 0 }
	};

	LuaObjectBase::CreateClass(s_type, l_parent, l_methods, 0, 0);
	LuaObjectBase::RegisterPromotion(l_parent, s_type, LuaObject<UI::Image>::DynamicCastPromotionTest);
}
