#include "List.h"
#include "LuaObject.h"

namespace UI {

class LuaList {
public:

};

}

static bool promotion_test(DeleteEmitter *o)
{
	return dynamic_cast<UI::List*>(o);
}

using namespace UI;

template <> const char *LuaObject<UI::List>::s_type = "UI.List";

template <> void LuaObject<UI::List>::RegisterClass()
{
	static const char *l_parent = "UI.Container";

	static const luaL_Reg l_methods[] = {

        { 0, 0 }
	};

	LuaObjectBase::CreateClass(s_type, l_parent, l_methods, 0, 0);
	LuaObjectBase::RegisterPromotion(l_parent, s_type, promotion_test);
}
