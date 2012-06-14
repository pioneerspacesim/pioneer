#include "Scroller.h"
#include "LuaObject.h"

namespace UI {

class LuaScroller {
public:

};

}

using namespace UI;

template <> const char *LuaObject<UI::Scroller>::s_type = "UI.Scroller";

template <> void LuaObject<UI::Scroller>::RegisterClass()
{
	static const char *l_parent = "UI.Container";

	static const luaL_Reg l_methods[] = {

        { 0, 0 }
	};

	LuaObjectBase::CreateClass(s_type, l_parent, l_methods, 0, 0);
	LuaObjectBase::RegisterPromotion(l_parent, s_type, LuaObject<UI::Scroller>::DynamicCastPromotionTest);
}
