// Copyright © 2008-2013 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#include "Gauge.h"
#include "Context.h"
#include "Skin.h"

namespace UI {

Point Gauge::PreferredSize()
{
	SetSizeControlFlags(EXPAND_WIDTH);
	return GetContext()->GetSkin().GaugeBackground().size;
}

void Gauge::Layout()
{
	SetActiveArea(Point(GetSize().x, GetContext()->GetSkin().GaugeBackground().size.y));
}

void Gauge::Draw()
{
	Context *c = GetContext();
	const Skin &s = c->GetSkin();
	Graphics::Renderer *r = c->GetRenderer();

	s.DrawGaugeBackground(GetActiveOffset(), GetActiveArea());

	glBlendFuncSeparate(GL_ZERO, GL_ONE, GL_SRC_COLOR, GL_ZERO);
	s.DrawGaugeMask(GetActiveOffset(), GetActiveArea());

	glBlendFunc(GL_DST_ALPHA, GL_ONE_MINUS_DST_ALPHA);
	s.DrawGaugeFill(GetActiveOffset(), GetActiveArea());

	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA); //restore default
}

}
