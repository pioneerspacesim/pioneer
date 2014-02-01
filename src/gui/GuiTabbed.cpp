// Copyright © 2008-2014 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#include "libs.h"
#include "Gui.h"
#include "GuiTabbed.h"

namespace Gui {

static const float TAB_BAR_HEIGHT = 20.0f;
static const float LABEL_PADDING  =	10.0f;

Tabbed::Tabbed()
{
	m_eventMask = EVENT_ALL;
	m_page = 0;
}

void Tabbed::AddPage(Widget *label, Widget *child)
{
	AppendChild(label, 0, 0);
	AppendChild(child, 0, TAB_BAR_HEIGHT);
	m_pages.push_back(std::pair<Widget*,Widget*>(label,child));
	if (m_page != m_pages.size()-1) child->Hide();
	label->Show();
	ShuffleLabels();
}

void Tabbed::Remove(Widget *child)
{
	for (pagecontainer_t::iterator i = m_pages.begin(); i!=m_pages.end(); ++i) {
		if ((*i).second == child) {
			RemoveChild((*i).first);
			RemoveChild((*i).second);
			m_pages.erase(i);
			return;
		}
	}
}

void Tabbed::GetSizeRequested(float size[2])
{
}

void Tabbed::SelectPage(int page)
{
	m_page = page;
	Show();
}

void Tabbed::OnActivate()
{
	SelectPage((m_page+1)%m_pages.size());
}

bool Tabbed::OnMouseDown(MouseButtonEvent *e)
{
	if ((e->button == SDL_BUTTON_LEFT) && (e->y < TAB_BAR_HEIGHT)) {
		float xpos = 0.0;
		int index = 0;
		for (pagecontainer_t::iterator i = m_pages.begin(); i!=m_pages.end();
				++i, index++) {
			float csize[2];
			(*i).first->GetSize(csize);
			csize[0] += 2*LABEL_PADDING;
			if (e->x - xpos < csize[0]) {
				SelectPage(index);
				onSelectPage.emit(index);
				break;
			}
			xpos += csize[0];
		}
		return false;
	} else {
		return Container::OnMouseDown(e);
	}
}

void Tabbed::ShuffleLabels()
{
	float xpos = LABEL_PADDING;
	int index=0;
	for (pagecontainer_t::iterator i = m_pages.begin(); i!=m_pages.end(); ++i, index++) {
		Container::MoveChild((*i).first, xpos, 0);
		float csize[2];
		(*i).first->GetSize(csize);
		xpos += csize[0] + 2*LABEL_PADDING;
	}
}

void Tabbed::OnChildResizeRequest(Widget *child)
{
	if (IsLabelWidget(child)) {
		float size[2], rsize[2];
		GetSize(size);
		rsize[0] = size[0];
		rsize[1] = TAB_BAR_HEIGHT;
		child->GetSizeRequested(rsize);
		rsize[0] = std::min(rsize[0], size[0]);
		rsize[1] = std::min(rsize[1], TAB_BAR_HEIGHT);
		child->SetSize(rsize[0], rsize[1]);
		ShuffleLabels();
	} else {
		float size[2], rsize[2];
		GetSize(size);
		rsize[0] = size[0];
		rsize[1] = size[1] - TAB_BAR_HEIGHT;
		child->GetSizeRequested(rsize);
		rsize[0] = std::min(rsize[0], size[0]);
		child->SetSize(rsize[0], rsize[1]);
	}
}

void Tabbed::UpdateAllChildSizes()
{
	for (pagecontainer_t::iterator i = m_pages.begin(); i != m_pages.end(); ++i) {
		OnChildResizeRequest((*i).first);
		OnChildResizeRequest((*i).second);
	}
}

void Tabbed::Show()
{
	unsigned int index=0;
	for (pagecontainer_t::iterator i = m_pages.begin(); i!=m_pages.end(); ++i, index++) {
		(*i).first->Show();
		if (index == m_page) (*i).second->Show();
		else (*i).second->Hide();
	}
	Container::Show();
}

void Tabbed::Hide()
{
	for (pagecontainer_t::iterator i = m_pages.begin(); i!=m_pages.end(); ++i) {
		(*i).first->Hide();
		(*i).second->Hide();
	}
	Widget::Hide();
}

bool Tabbed::IsLabelWidget(const Widget *w)
{
	for (pagecontainer_t::iterator i = m_pages.begin(); i!=m_pages.end(); ++i) {
		if ((*i).first == w) return true;
	}
	return false;
}

void Tabbed::Draw()
{
	PROFILE_SCOPED()
	float size[2];
	GetSize(size);
	float xpos = 0;
	unsigned int index = 0;

	Theme::DrawRect(vector2f(0.f), vector2f(size[0], TAB_BAR_HEIGHT), Theme::Colors::bgShadow, Screen::alphaBlendState);

	for (pagecontainer_t::iterator i = m_pages.begin(); i!=m_pages.end();
			++i, index++) {
		float csize[2];
		(*i).first->GetSize(csize);
		csize[0] += 2*LABEL_PADDING;
		if (index == m_page) {
			Theme::DrawRect(vector2f(xpos, 0.f), vector2f(xpos+csize[0], TAB_BAR_HEIGHT), Theme::Colors::bg, Screen::alphaBlendState);
		}
		xpos += csize[0];
	}
	//AppendChild(label, m_pages.size()*50 + (2*m_pages.size()+1)*LABEL_PADDING, 0);
	Container::Draw();
}

}
