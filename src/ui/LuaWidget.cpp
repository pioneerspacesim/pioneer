#include "Widget.h"
#include "LuaObject.h"
#include "LuaConstants.h"

namespace UI {

class LuaWidget {
public:

	static int l_set_font_size(lua_State *l) {
		UI::Widget *w = LuaObject<UI::Widget>::CheckFromLua(1);
		UI::Widget::FontSize fontSize = static_cast<UI::Widget::FontSize>(LuaConstants::GetConstantFromArg(l, "UIFontSize", 2));
		w->SetFontSize(fontSize);
		lua_pushvalue(l, 1);
		return 1;
	}

};

}

using namespace UI;

template <> const char *LuaObject<UI::Widget>::s_type = "UI.Widget";

template <> void LuaObject<UI::Widget>::RegisterClass()
{
	static const luaL_Reg l_methods[] = {
		{ "SetFontSize", LuaWidget::l_set_font_size },
		{ 0, 0 }
	};

	LuaObjectBase::CreateClass(s_type, 0, l_methods, 0, 0);
}
