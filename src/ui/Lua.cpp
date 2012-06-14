#include "Lua.h"

namespace UI {

void LuaInit()
{
	LuaObject<UI::Align>::RegisterClass();
	LuaObject<UI::Background>::RegisterClass();
	LuaObject<UI::Box>::RegisterClass();
	LuaObject<UI::HBox>::RegisterClass();
	LuaObject<UI::VBox>::RegisterClass();
	LuaObject<UI::Button>::RegisterClass();
	LuaObject<UI::CheckBox>::RegisterClass();
	LuaObject<UI::ColorBackground>::RegisterClass();
	LuaObject<UI::Container>::RegisterClass();
	LuaObject<UI::Context>::RegisterClass();
	LuaObject<UI::DropDown>::RegisterClass();
	LuaObject<UI::Grid>::RegisterClass();
	LuaObject<UI::Image>::RegisterClass();
	LuaObject<UI::Label>::RegisterClass();
	LuaObject<UI::List>::RegisterClass();
	LuaObject<UI::Margin>::RegisterClass();
	LuaObject<UI::MultiLineText>::RegisterClass();
	LuaObject<UI::Scroller>::RegisterClass();
	LuaObject<UI::Single>::RegisterClass();
	LuaObject<UI::Slider>::RegisterClass();
	LuaObject<UI::HSlider>::RegisterClass();
	LuaObject<UI::VSlider>::RegisterClass();
	LuaObject<UI::Widget>::RegisterClass();
}

}
