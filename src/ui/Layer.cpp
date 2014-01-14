// Copyright © 2008-2014 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#include "Layer.h"
#include "Context.h"

namespace UI {

void Layer::Layout()
{
	LayoutChildren();
}

Layer *Layer::SetInnerWidget(Widget *w, const Point &pos, const Point &size)
{
	assert(!w->GetContainer());

	Container::RemoveAllWidgets();
	m_widget.Reset(w);

	Container::AddWidget(w);
	Container::SetWidgetDimensions(w, pos, size);

	GetContext()->RequestLayout();

	return this;
}

void Layer::RemoveInnerWidget()
{
	Container::RemoveAllWidgets();
	m_widget.Reset(0);

	GetContext()->RequestLayout();
}

}
