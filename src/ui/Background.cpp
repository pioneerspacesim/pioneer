// Copyright © 2008-2014 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#include "Background.h"
#include "Context.h"
#include "Skin.h"

namespace UI {

Point Background::PreferredSize()
{
	const Skin::BorderedRectElement &elem(GetContext()->GetSkin().BackgroundNormal());
	const Point borderSize(elem.borderWidth*2, elem.borderHeight*2);
	if (!GetInnerWidget()) return borderSize;
	return SizeAdd(GetInnerWidget()->PreferredSize(), borderSize);
}

void Background::Layout()
{
	if (!GetInnerWidget()) return;
	const Skin::BorderedRectElement &elem(GetContext()->GetSkin().BackgroundNormal());
	SetWidgetDimensions(GetInnerWidget(), Point(elem.borderWidth, elem.borderHeight), GetSize()-Point(elem.borderWidth*2, elem.borderHeight*2));
	return GetInnerWidget()->Layout();
}

void Background::Draw()
{
	GetContext()->GetSkin().DrawBackgroundNormal(Point(), GetSize());
	Single::Draw();
}

}
