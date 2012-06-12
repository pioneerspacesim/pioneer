#include "Widget.h"
#include "LuaObject.h"

namespace UI {

class LuaWidget {
public:

};

}

using namespace UI;

template <> const char *LuaObject<UI::Widget>::s_type = "UI.Widget";

template <> void LuaObject<UI::Widget>::RegisterClass()
{
	static const luaL_Reg l_methods[] = {

        { 0, 0 }
	};

	LuaObjectBase::CreateClass(s_type, 0, l_methods, 0, 0);
}
