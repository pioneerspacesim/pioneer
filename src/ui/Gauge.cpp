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
	const Point activeOffset(GetActiveOffset());
	const Point activeArea(GetActiveArea());

	Context *c = GetContext();
	const Skin &s = c->GetSkin();

	s.DrawGaugeBackground(activeOffset, activeArea);

	if (m_value > 0.0f) {
		Graphics::Renderer *r = c->GetRenderer();

		r->SetBlendMode(Graphics::BLEND_SET_ALPHA);
		s.DrawGaugeMask(activeOffset, activeArea);

		r->SetBlendMode(Graphics::BLEND_DEST_ALPHA);
		s.DrawGaugeFill(activeOffset, Point(activeArea.x * m_value, activeArea.y));

		r->SetBlendMode(Graphics::BLEND_ALPHA); // restore default
	}
}

}
