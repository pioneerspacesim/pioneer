#include "Container.h"
#include "LuaObject.h"

namespace UI {

class LuaContainer {
public:

};

}

static bool promotion_test(DeleteEmitter *o)
{
	return dynamic_cast<UI::Container*>(o);
}

using namespace UI;

template <> const char *LuaObject<UI::Container>::s_type = "UI.Container";

template <> void LuaObject<UI::Container>::RegisterClass()
{
	static const char *l_parent = "UI.Widget";

	static const luaL_Reg l_methods[] = {

        { 0, 0 }
	};

	LuaObjectBase::CreateClass(s_type, l_parent, l_methods, 0, 0);
	LuaObjectBase::RegisterPromotion(l_parent, s_type, promotion_test);
}
