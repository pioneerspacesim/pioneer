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
	GetContext()->GetSkin().DrawGaugeBorder(GetActiveOffset(), GetActiveArea());
}

}
