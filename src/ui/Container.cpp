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
	assert(!widget->GetContainer());

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

void Container::SetWidgetDimensions(Widget *widget, const Point &position, const Point &size)
{
	assert(widget->GetContainer() == this);

	widget->SetDimensions(position, size);
}

Widget *Container::GetWidgetAtAbsolute(const Point &pos)
{
	if (!ContainsAbsolute(pos)) return 0;

	for (WidgetIterator i = WidgetsBegin(); i != WidgetsEnd(); ++i) {
		Widget *widget = (*i).Get();
		if (widget->ContainsAbsolute(pos)) {
			if (widget->IsContainer())
				return static_cast<Container*>(widget)->GetWidgetAtAbsolute(pos);
			else
				return widget;
		}
	}
	
	return this;
}

}
