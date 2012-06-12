#include "Scroller.h"
#include "LuaObject.h"

namespace UI {

class LuaScroller {
public:

};

}

static bool promotion_test(DeleteEmitter *o)
{
	return dynamic_cast<UI::Scroller*>(o);
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
	LuaObjectBase::RegisterPromotion(l_parent, s_type, promotion_test);
}
