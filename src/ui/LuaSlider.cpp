#include "Slider.h"
#include "LuaObject.h"

namespace UI {

class LuaSlider {
public:

};

}

static bool promotion_test(DeleteEmitter *o)
{
	return dynamic_cast<UI::Slider*>(o);
}

using namespace UI;

template <> const char *LuaObject<UI::Slider>::s_type = "UI.Slider";

template <> void LuaObject<UI::Slider>::RegisterClass()
{
	static const char *l_parent = "UI.Slider";

	static const luaL_Reg l_methods[] = {

        { 0, 0 }
	};

	LuaObjectBase::CreateClass(s_type, l_parent, l_methods, 0, 0);
	LuaObjectBase::RegisterPromotion(l_parent, s_type, promotion_test);
}
