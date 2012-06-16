#include "Box.h"
#include "LuaObject.h"

namespace UI {

class LuaBox {
public:

};

class LuaHBox;
class LuaVBox;

}

using namespace UI;

template <> const char *LuaObject<UI::Box>::s_type = "UI.Box";

template <> void LuaObject<UI::Box>::RegisterClass()
{
	static const char *l_parent = "UI.Container";

	static const luaL_Reg l_methods[] = {

        { 0, 0 }
	};

	LuaObjectBase::CreateClass(s_type, l_parent, l_methods, 0, 0);
	LuaObjectBase::RegisterPromotion(l_parent, s_type, LuaObject<UI::Box>::DynamicCastPromotionTest);
}

template <> const char *LuaObject<UI::HBox>::s_type = "UI.HBox";

template <> void LuaObject<UI::HBox>::RegisterClass()
{
	static const char *l_parent = "UI.Box";

	LuaObjectBase::CreateClass(s_type, l_parent, 0, 0, 0);
	LuaObjectBase::RegisterPromotion(l_parent, s_type, LuaObject<UI::HBox>::DynamicCastPromotionTest);
}

template <> const char *LuaObject<UI::VBox>::s_type = "UI.VBox";

template <> void LuaObject<UI::VBox>::RegisterClass()
{
	static const char *l_parent = "UI.Box";

	LuaObjectBase::CreateClass(s_type, l_parent, 0, 0, 0);
	LuaObjectBase::RegisterPromotion(l_parent, s_type, LuaObject<UI::VBox>::DynamicCastPromotionTest);
}
