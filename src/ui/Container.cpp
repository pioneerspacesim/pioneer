// Copyright © 2008-2012 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#include "Container.h"
#include "Context.h"
#include "matrix4x4.h"
#include "graphics/Renderer.h"

namespace UI {

Container::~Container()
{
	for (std::vector< RefCountedPtr<Widget> >::iterator i = m_widgets.begin(); i != m_widgets.end(); ++i)
		(*i)->Detach();
}

void Container::Update()
{
	for (std::vector< RefCountedPtr<Widget> >::iterator i = m_widgets.begin(); i != m_widgets.end(); ++i)
		(*i)->Update();
}

void Container::Draw()
{
	Context *c = GetContext();

	for (std::vector< RefCountedPtr<Widget> >::iterator i = m_widgets.begin(); i != m_widgets.end(); ++i)
		c->DrawWidget((*i).Get());
}

void Container::LayoutChildren()
{
	for (std::vector< RefCountedPtr<Widget> >::iterator i = m_widgets.begin(); i != m_widgets.end(); ++i)
		(*i)->Layout();
}

void Container::AddWidget(Widget *widget)
{
	if (widget->GetContainer())
		widget->GetContainer()->RemoveWidget(widget);

	std::vector< RefCountedPtr<Widget> >::iterator i;
	for (i = m_widgets.begin(); i != m_widgets.end(); ++i)
		if ((*i).Get() == widget) break;
	assert(i == m_widgets.end());

	widget->Attach(this);
	m_widgets.push_back(RefCountedPtr<Widget>(widget));

	GetContext()->RequestLayout();
}

void Container::RemoveWidget(Widget *widget)
{
	assert(widget->GetContainer() == this);

	std::vector< RefCountedPtr<Widget> >::iterator i;
	for (i = m_widgets.begin(); i != m_widgets.end(); ++i)
		if ((*i).Get() == widget) break;
	if (i == m_widgets.end())
		return;

	widget->Detach();
	m_widgets.erase(i);

	GetContext()->RequestLayout();
}

void Container::RemoveAllWidgets()
{
	std::vector< RefCountedPtr<Widget> >::iterator i = m_widgets.begin();
	while (i != m_widgets.end()) {
		(*i)->Detach();
		i = m_widgets.erase(i);
	}

	GetContext()->RequestLayout();
}

void Container::Disable()
{
	DisableChildren();
	Widget::Disable();
}

void Container::Enable()
{
	EnableChildren();
	Widget::Enable();
}

void Container::DisableChildren()
{
	for (std::vector< RefCountedPtr<Widget> >::iterator i = m_widgets.begin(); i != m_widgets.end(); ++i) {
		Widget *w = (*i).Get();
		w->SetDisabled(true);
		Container *c = dynamic_cast<Container*>(w);
		if (c) c->DisableChildren();
	}
}

void Container::EnableChildren()
{
	for (std::vector< RefCountedPtr<Widget> >::iterator i = m_widgets.begin(); i != m_widgets.end(); ++i) {
		Widget *w = (*i).Get();
		w->SetDisabled(false);
		Container *c = dynamic_cast<Container*>(w);
		if (c) c->EnableChildren();
	}
}

Point Container::CalcLayoutContribution(Widget *w)
{
	Point preferredSize = w->PreferredSize();
	const Uint32 flags = w->GetSizeControlFlags();

	if (flags & NO_WIDTH)
		preferredSize.x = 0;
	if (flags & NO_HEIGHT)
		preferredSize.y = 0;

	if (flags & EXPAND_WIDTH)
		preferredSize.x = SIZE_EXPAND;
	if (flags & EXPAND_HEIGHT)
		preferredSize.y = SIZE_EXPAND;

	return preferredSize;
}

Point Container::CalcSize(Widget *w, const Point &avail)
{
	if (!(w->GetSizeControlFlags() & PRESERVE_ASPECT))
		return avail;

	const Point preferredSize = w->PreferredSize();

	float wantRatio = float(preferredSize.x) / float(preferredSize.y);

	// more room on X than Y, use full X, scale Y
	if (avail.x > avail.y)
		return Point(float(avail.y) * wantRatio, avail.y);

	// more room on Y than X, use full Y, scale X
	else
		return Point(avail.x, float(avail.x) / wantRatio);
}

void Container::SetWidgetDimensions(Widget *widget, const Point &position, const Point &size)
{
	assert(widget->GetContainer() == this);

	widget->SetDimensions(position, size);
}

Widget *Container::GetWidgetAt(const Point &pos)
{
	if (!Contains(pos)) return 0;

	for (WidgetIterator i = WidgetsBegin(); i != WidgetsEnd(); ++i) {
		Widget *widget = (*i).Get();
		const Point relpos = pos - widget->GetPosition() - widget->GetDrawOffset();
		if (widget->IsContainer()) {
			Widget* w = static_cast<Container*>(widget)->GetWidgetAt(relpos);
			if (w) return w;
		} else if (widget->Contains(relpos))
			return widget;
	}

	return this;
}

}
