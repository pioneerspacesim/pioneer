// Copyright © 2008-2013 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#include "Gui.h"
#include "GuiContainer.h"

//#define GUI_DEBUG_CONTAINER

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
	for (std::list<widget_pos>::iterator i = m_children.begin(); i != m_children.end(); ++i) {
		if ((*i).w->IsMouseOver() == true)
			(*i).w->OnMouseLeave();
	}
}

bool Container::OnMouseMotion(MouseMotionEvent *e)
{
	float x = e->x;
	float y = e->y;
	for (std::list<widget_pos>::iterator i = m_children.begin(); i != m_children.end(); ++i) {
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
	for (std::list<widget_pos>::iterator i = m_children.begin(); i != m_children.end(); ++i) {
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
	for (std::list<widget_pos>::iterator i = m_children.begin(); i != m_children.end(); ++i) {
		delete (*i).w;
	}
	m_children.clear();
}

void Container::RemoveAllChildren()
{
	for (std::list<widget_pos>::iterator i = m_children.begin(); i != m_children.end(); ++i) {
		i->w->SetParent(0);
	}
	m_children.clear();
}

void Container::PrependChild(Widget *child, float x, float y)
{
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
	std::list<widget_pos>::iterator it = FindChild(child);
	if (it != m_children.end()) {
		it->pos[0] = x;
		it->pos[1] = y;
	}
}

void Container::RemoveChild(Widget *child)
{
	std::list<widget_pos>::iterator it = FindChild(child);
	if (it != m_children.end()) {
		it->w->SetParent(0);
		m_children.erase(it);
	}
}

Container::WidgetList::const_iterator Container::FindChild(const Widget *w) const
{
	for (std::list<widget_pos>::const_iterator i = m_children.begin(); i != m_children.end(); ++i)
		if (i->w == w) return i;
	return m_children.end();
}

Container::WidgetList::iterator Container::FindChild(const Widget *w)
{
	for (std::list<widget_pos>::iterator i = m_children.begin(); i != m_children.end(); ++i)
		if (i->w == w) return i;
	return m_children.end();
}

void Container::Draw()
{
	PROFILE_SCOPED()
	float size[2];
	GetSize(size);
	if (!m_transparent) {
		if (m_bgcol[3] < 1.0) {
			glEnable(GL_BLEND);
			glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		}
		glBegin(GL_QUADS);
			glColor4fv(m_bgcol);
			glVertex2f(0, size[1]);
			glVertex2f(size[0], size[1]);
			glVertex2f(size[0], 0);
			glVertex2f(0, 0);
		glEnd();
		if (m_bgcol[3] < 1.0) {
			glBlendFunc(GL_ONE, GL_ZERO);
			glDisable(GL_BLEND);
		}
	}
#ifdef GUI_DEBUG_CONTAINER
	glBegin(GL_LINE_LOOP);
		glColor3f(1,1,1);
		glVertex2f(0, size[1]);
		glVertex2f(size[0], size[1]);
		glVertex2f(size[0], 0);
		glVertex2f(0, 0);
	glEnd();
#endif /* GUI_DEBUG_CONTAINER */
	for (std::list<widget_pos>::iterator i = m_children.begin(); i != m_children.end(); ++i) {
		if (!(*i).w->IsVisible()) continue;
		glPushMatrix();
		glTranslatef((*i).pos[0], (*i).pos[1], 0);
#ifdef GUI_DEBUG_CONTAINER
		float csize[2];
		(*i).w->GetSize(csize);

		glBegin(GL_LINE_LOOP);
			glColor3f(0,0,1);
			glVertex2f(0, csize[1]);
			glVertex2f(csize[0], csize[1]);
			glVertex2f(csize[0], 0);
			glVertex2f(0, 0);
		glEnd();
#endif /* GUI_DEBUG_CONTAINER */
		(*i).w->Draw();
		glPopMatrix();
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
	for (std::list<widget_pos>::iterator i = m_children.begin(); i != m_children.end(); ++i) {
		(*i).w->Show();
	}
}

void Container::HideChildren()
{
	for (std::list<widget_pos>::iterator i = m_children.begin(); i != m_children.end(); ++i) {
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
	Widget::Show();
	if (IsVisible()) {
		ResizeRequest();
	}
}

void Container::ShowAll()
{
	for (std::list<widget_pos>::iterator i = m_children.begin(); i != m_children.end(); ++i) {
		(*i).w->ShowAll();
	}
	Show();
}

void Container::HideAll()
{
	HideChildren();
	Hide();
}

void Container::SetBgColor(const Color &col)
{
	m_bgcol = col;
}

void Container::SetBgColor(float r, float g, float b, float a)
{
	m_bgcol = Color(r, g, b, a);
}

}
