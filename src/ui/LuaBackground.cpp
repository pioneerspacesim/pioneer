#include "Background.h"
#include "LuaObject.h"

namespace UI {

class LuaBackground {
public:

};

}

static bool promotion_test(DeleteEmitter *o)
{
	return dynamic_cast<UI::Background*>(o);
}

using namespace UI;

template <> const char *LuaObject<UI::Background>::s_type = "UI.Background";

template <> void LuaObject<UI::Background>::RegisterClass()
{
	static const char *l_parent = "UI.Single";

	static const luaL_Reg l_methods[] = {

        { 0, 0 }
	};

	LuaObjectBase::CreateClass(s_type, l_parent, l_methods, 0, 0);
	LuaObjectBase::RegisterPromotion(l_parent, s_type, promotion_test);
}
