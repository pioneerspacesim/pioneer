#include "DropDown.h"
#include "LuaObject.h"

namespace UI {

class LuaDropDown {
public:

};

}

using namespace UI;

template <> const char *LuaObject<UI::DropDown>::s_type = "UI.DropDown";

template <> void LuaObject<UI::DropDown>::RegisterClass()
{
	static const char *l_parent = "UI.Widget";

	static const luaL_Reg l_methods[] = {

        { 0, 0 }
	};

	LuaObjectBase::CreateClass(s_type, l_parent, l_methods, 0, 0);
	LuaObjectBase::RegisterPromotion(l_parent, s_type, LuaObject<UI::DropDown>::DynamicCastPromotionTest);
}
