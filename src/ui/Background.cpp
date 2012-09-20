#include "Background.h"
#include "Context.h"
#include "Skin.h"

namespace UI {

vector2f Background::PreferredSize()
{
	const vector2f borderSize(Skin::s_backgroundNormal.borderWidth*2);
	if (!GetInnerWidget()) return borderSize;
	return GetInnerWidget()->PreferredSize() + borderSize;
}

void Background::Layout()
{
	if (!GetInnerWidget()) return;
	SetWidgetDimensions(GetInnerWidget(), vector2f(Skin::s_backgroundNormal.borderWidth), GetSize()-vector2f(Skin::s_backgroundNormal.borderWidth*2));
	return GetInnerWidget()->Layout();
}

void Background::Draw()
{
	GetContext()->GetSkin().DrawBackgroundNormal(vector2f(), GetSize());
	Single::Draw();
}

}
