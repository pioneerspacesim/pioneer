#include "Grid.h"
#include "LuaObject.h"

namespace UI {

class LuaGrid {
public:

};

}

using namespace UI;

template <> const char *LuaObject<UI::Grid>::s_type = "UI.Grid";

template <> void LuaObject<UI::Grid>::RegisterClass()
{
	static const char *l_parent = "UI.Container";

	static const luaL_Reg l_methods[] = {

        { 0, 0 }
	};

	LuaObjectBase::CreateClass(s_type, l_parent, l_methods, 0, 0);
	LuaObjectBase::RegisterPromotion(l_parent, s_type, LuaObject<UI::Grid>::DynamicCastPromotionTest);
}
