#include "Slider.h"
#include "LuaObject.h"

namespace UI {

class LuaSlider {
public:

};

class LuaHSlider;
class LuaVSlider;

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
	LuaObjectBase::RegisterPromotion(l_parent, s_type, LuaObject<UI::Slider>::DynamicCastPromotionTest);
}

template <> const char *LuaObject<UI::HSlider>::s_type = "UI.HSlider";

template <> void LuaObject<UI::HSlider>::RegisterClass()
{
	static const char *l_parent = "UI.Slider";

	LuaObjectBase::CreateClass(s_type, l_parent, 0, 0, 0);
	LuaObjectBase::RegisterPromotion(l_parent, s_type, LuaObject<UI::HSlider>::DynamicCastPromotionTest);
}

template <> const char *LuaObject<UI::VSlider>::s_type = "UI.VSlider";

template <> void LuaObject<UI::VSlider>::RegisterClass()
{
	static const char *l_parent = "UI.Slider";

	LuaObjectBase::CreateClass(s_type, l_parent, 0, 0, 0);
	LuaObjectBase::RegisterPromotion(l_parent, s_type, LuaObject<UI::VSlider>::DynamicCastPromotionTest);
}
