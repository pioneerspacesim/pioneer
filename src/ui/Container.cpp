// Copyright © 2008-2013 Pioneer Developers. See AUTHORS.txt for details
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

void Container::CollectShortcuts(std::map<KeySym,Widget*> &shortcuts)
{
	{
	const std::set<KeySym> &widgetShortcuts = GetShortcuts();
	if (!widgetShortcuts.empty())
		for (std::set<KeySym>::const_iterator j = widgetShortcuts.begin(); j != widgetShortcuts.end(); ++j)
			shortcuts[*j] = this;
	}

	for (WidgetIterator i = WidgetsBegin(); i != WidgetsEnd(); ++i) {
		Widget *widget = (*i).Get();
		if (widget->IsContainer())
			static_cast<Container*>(widget)->CollectShortcuts(shortcuts);
		else {
			const std::set<KeySym> &widgetShortcuts = widget->GetShortcuts();
			if (!widgetShortcuts.empty())
				for (std::set<KeySym>::const_iterator j = widgetShortcuts.begin(); j != widgetShortcuts.end(); ++j)
					shortcuts[*j] = widget;
		}
	}
}

}
