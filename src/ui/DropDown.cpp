#include "DropDown.h"

namespace UI {

DropDown::DropDown(Context *context) : Widget(context)
{
}

vector2f DropDown::PreferredSize()
{
	return 0;
}

void DropDown::Layout()
{
}

void DropDown::Draw()
{
}

DropDown *DropDown::AddOption(const std::string &text)
{
	return this;
}

}
