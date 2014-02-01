// Copyright © 2008-2014 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#include "Gui.h"
#include "GuiContainer.h"

#include <SDL_stdinc.h>

namespace Gui {

Container::Container()
{
	m_transparent = true;
	SetBgColor(Theme::Colors::bg);
	onMouseLeave.connect(sigc::mem_fun(this, &Container::_OnMouseLeave));
	onSetSize.connect(sigc::mem_fun(this, &Container::_OnSetSize));
}

Container::~Container()
{
	DeleteAllChildren();
}

void Container::_OnSetSize()
{
	if (IsVisible()) UpdateAllChildSizes();
}

void Container::_OnMouseLeave()
{
	for (WidgetList::iterator i = m_children.begin(), itEnd = m_children.end(); i != itEnd; ++i) {
		if ((*i).w->IsMouseOver() == true)
			(*i).w->OnMouseLeave();
	}
}

bool Container::OnMouseMotion(MouseMotionEvent *e)
{
	float x = e->x;
	float y = e->y;
	for (WidgetList::iterator i = m_children.begin(), itEnd = m_children.end(); i != itEnd; ++i) {
		float *pos,size[2];
		if (!(*i).w->IsVisible()) {
			if ((*i).w->IsMouseOver() == true)
				(*i).w->OnMouseLeave();
			continue;
		}
		int evmask = (*i).w->GetEventMask();
		if (!(evmask & Widget::EVENT_MOUSEMOTION)) continue;

		pos = (*i).pos;
		(*i).w->GetSize(size);

		if ((x >= pos[0]) && (x < pos[0]+size[0]) &&
		    (y >= pos[1]) && (y < pos[1]+size[1])) {
			e->x = x-pos[0];
			e->y = y-pos[1];
			if ((*i).w->IsMouseOver() == false) {
				(*i).w->OnMouseEnter();
			}
			bool alive = (*i).w->OnMouseMotion(e);
			if (!alive) return false;
		} else {
			if ((*i).w->IsMouseOver() == true)
				(*i).w->OnMouseLeave();
		}
	}
	return true;
}

bool Container::HandleMouseEvent(MouseButtonEvent *e)
{
	float x = e->x;
	float y = e->y;
	for (WidgetList::iterator i = m_children.begin(), itEnd = m_children.end(); i != itEnd; ++i) {
		float *pos,size[2];
		if (!(*i).w->IsVisible()) continue;
		if (!(*i).w->GetEnabled()) continue;
		int evmask = (*i).w->GetEventMask();
		if (e->isdown) {
			if (!(evmask & Widget::EVENT_MOUSEDOWN)) continue;
		} else {
			if (!(evmask & Widget::EVENT_MOUSEUP)) continue;
		}
		pos = (*i).pos;
		(*i).w->GetSize(size);

		if ((x >= pos[0]) && (x < pos[0]+size[0]) &&
		    (y >= pos[1]) && (y < pos[1]+size[1])) {
			e->x = x-pos[0];
			e->y = y-pos[1];
			bool alive;
			if (e->isdown) {
				alive = (*i).w->OnMouseDown(e);
			} else {
				alive = (*i).w->OnMouseUp(e);
			}
			if (!alive) return false;
		}
	}
	onMouseButtonEvent.emit(e);
	return true;
}

void Container::DeleteAllChildren()
{
	PROFILE_SCOPED()
	for (WidgetList::iterator i = m_children.begin(), itEnd = m_children.end(); i != itEnd; ++i) {
		delete (*i).w;
	}
	m_children.clear();
}

void Container::RemoveAllChildren()
{
	PROFILE_SCOPED()
	for (WidgetList::iterator i = m_children.begin(), itEnd = m_children.end(); i != itEnd; ++i) {
		i->w->SetParent(0);
	}
	m_children.clear();
}

void Container::PrependChild(Widget *child, float x, float y)
{
	PROFILE_SCOPED()
	assert(child->GetParent() == 0);
	assert(FindChild(child) == m_children.end());

	widget_pos wp;
	wp.w = child;
	wp.pos[0] = x; wp.pos[1] = y;
	wp.flags = 0;
	child->SetParent(this);
	m_children.push_front(wp);
}

void Container::AppendChild(Widget *child, float x, float y)
{
	PROFILE_SCOPED()
	assert(child->GetParent() == 0);
	assert(FindChild(child) == m_children.end());

	widget_pos wp;
	wp.w = child;
	wp.pos[0] = x; wp.pos[1] = y;
	wp.flags = 0;
	child->SetParent(this);
	m_children.push_back(wp);
}

void Container::MoveChild(Widget *child, float x, float y)
{
	PROFILE_SCOPED()
	WidgetList::iterator it = FindChild(child);
	if (it != m_children.end()) {
		it->pos[0] = x;
		it->pos[1] = y;
	}
}

void Container::RemoveChild(Widget *child)
{
	PROFILE_SCOPED()
	WidgetList::iterator it = FindChild(child);
	if (it != m_children.end()) {
		it->w->SetParent(0);
		m_children.erase(it);
	}
}

Container::WidgetList::const_iterator Container::FindChild(const Widget *w) const
{
	PROFILE_SCOPED()
	for (WidgetList::const_iterator i = m_children.begin(); i != m_children.end(); ++i)
		if (i->w == w) return i;
	return m_children.end();
}

Container::WidgetList::iterator Container::FindChild(const Widget *w)
{
	PROFILE_SCOPED()
	for (WidgetList::iterator i = m_children.begin(), itEnd = m_children.end(); i != itEnd; ++i)
		if (i->w == w) return i;
	return m_children.end();
}

void Container::Draw()
{
	PROFILE_SCOPED()

	Graphics::Renderer *r = Gui::Screen::GetRenderer();
	r->SetRenderState(Gui::Screen::alphaBlendState);

	float size[2];
	GetSize(size);
	if (!m_transparent) {
		PROFILE_SCOPED_RAW("Container::Draw - !m_transparent")
		Theme::DrawRect(vector2f(0.f), vector2f(size[0], size[1]), m_bgcol, Screen::alphaBlendState);
	}

	for (WidgetList::iterator i = m_children.begin(), itEnd = m_children.end(); i != itEnd; ++i) {
		if (!(*i).w->IsVisible()) continue;

		PROFILE_SCOPED_RAW("Container::Draw - Child Loop")

		Graphics::Renderer::MatrixTicket ticket(r, Graphics::MatrixMode::MODELVIEW);
		r->Translate((*i).pos[0], (*i).pos[1], 0);
		(*i).w->Draw();
	}
}

bool Container::OnMouseDown(MouseButtonEvent *e)
{
	return HandleMouseEvent(e);
}

bool Container::OnMouseUp(MouseButtonEvent *e)
{
	return HandleMouseEvent(e);
}

void Container::ShowChildren()
{
	PROFILE_SCOPED()
	for (WidgetList::iterator i = m_children.begin(), itEnd = m_children.end(); i != itEnd; ++i) {
		(*i).w->Show();
	}
}

void Container::HideChildren()
{
	PROFILE_SCOPED()
	for (WidgetList::iterator i = m_children.begin(), itEnd = m_children.end(); i != itEnd; ++i) {
		(*i).w->Hide();
	}
}

void Container::GetChildPosition(const Widget *child, float outPos[2]) const
{
	WidgetList::const_iterator it = FindChild(child);
	assert(it != m_children.end());
	outPos[0] = it->pos[0];
	outPos[1] = it->pos[1];
}

void Container::Show()
{
	PROFILE_SCOPED()
	Widget::Show();
	if (IsVisible()) {
		ResizeRequest();
	}
}

void Container::ShowAll()
{
	PROFILE_SCOPED()
	for (WidgetList::iterator i = m_children.begin(), itEnd = m_children.end(); i != itEnd; ++i) {
		(*i).w->ShowAll();
	}
	Show();
}

void Container::HideAll()
{
	PROFILE_SCOPED()
	HideChildren();
	Hide();
}

void Container::SetBgColor(const Color &col)
{
	m_bgcol = col;
}

}
