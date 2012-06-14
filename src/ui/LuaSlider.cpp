#include "Slider.h"
#include "LuaObject.h"

namespace UI {

class LuaSlider {
public:

};

class LuaHSlider;
class LuaVSlider;

}

static bool slider_promotion_test(DeleteEmitter *o)
{
	return dynamic_cast<UI::Slider*>(o);
}

static bool hslider_promotion_test(DeleteEmitter *o)
{
	return dynamic_cast<UI::HSlider*>(o);
}

static bool vslider_promotion_test(DeleteEmitter *o)
{
	return dynamic_cast<UI::VSlider*>(o);
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
	LuaObjectBase::RegisterPromotion(l_parent, s_type, slider_promotion_test);
}

template <> const char *LuaObject<UI::HSlider>::s_type = "UI.HSlider";

template <> void LuaObject<UI::HSlider>::RegisterClass()
{
	static const char *l_parent = "UI.Slider";

	LuaObjectBase::CreateClass(s_type, l_parent, 0, 0, 0);
	LuaObjectBase::RegisterPromotion(l_parent, s_type, hslider_promotion_test);
}

template <> const char *LuaObject<UI::VSlider>::s_type = "UI.VSlider";

template <> void LuaObject<UI::VSlider>::RegisterClass()
{
	static const char *l_parent = "UI.Slider";

	LuaObjectBase::CreateClass(s_type, l_parent, 0, 0, 0);
	LuaObjectBase::RegisterPromotion(l_parent, s_type, vslider_promotion_test);
}
