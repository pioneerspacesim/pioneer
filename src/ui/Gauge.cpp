// Copyright © 2008-2013 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#include "Gauge.h"
#include "Context.h"
#include "Skin.h"

namespace UI {

Point Gauge::PreferredSize()
{
	return GetContext()->GetSkin().GaugeBorder().size;
}

void Gauge::Layout()
{
	SetActiveArea(PreferredSize());
}

void Gauge::Draw()
{
	Context *c = GetContext();
	const Skin &s = c->GetSkin();
	Graphics::Renderer *r = c->GetRenderer();

	glEnable(GL_BLEND);

	glBlendFunc(GL_ONE, GL_ZERO);
	s.DrawGaugeBorder(GetActiveOffset(), GetActiveArea());

	glBlendFuncSeparate(GL_ZERO, GL_ONE, GL_SRC_COLOR, GL_ZERO);
	s.DrawGaugeMask(GetActiveOffset(), GetActiveArea());

	glBlendFunc(GL_DST_ALPHA, GL_ONE_MINUS_DST_ALPHA);
	s.DrawGaugeFill(GetActiveOffset(), GetActiveArea());
}

}
