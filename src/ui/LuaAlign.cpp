#include "Align.h"
#include "LuaObject.h"

namespace UI {

class LuaAlign {
public:

};

}

static bool promotion_test(DeleteEmitter *o)
{
	return dynamic_cast<UI::Align*>(o);
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
	LuaObjectBase::RegisterPromotion(l_parent, s_type, promotion_test);
}
