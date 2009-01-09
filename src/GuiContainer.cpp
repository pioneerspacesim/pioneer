#include "Gui.h"
#include "GuiContainer.h"

namespace Gui {

Container::Container()
{
	onMouseLeave.connect(sigc::mem_fun(this, &Container::_OnMouseLeave));
	onSetSize.connect(sigc::mem_fun(this, &Container::_OnSetSize));
}

Container::~Container()
{
	DeleteAllChildren();
}

void Container::_OnSetSize()
{
	for (std::list<widget_pos>::iterator i = m_children.begin(); i != m_children.end(); ++i) {
		OnChildResizeRequest((*i).w);
	}
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
		float pos[2],size[2];
		if (!(*i).w->IsVisible()) {
			if ((*i).w->IsMouseOver() == true)
				(*i).w->OnMouseLeave();
			continue;
		}
		int evmask = (*i).w->GetEventMask();
		if (!(evmask & Widget::EVENT_MOUSEMOTION)) continue;

		(*i).w->GetPosition(pos);
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
		float pos[2],size[2];
		if (!(*i).w->IsVisible()) continue;
		if (!(*i).w->GetEnabled()) continue;
		int evmask = (*i).w->GetEventMask();
		if (e->isdown) {
			if (!(evmask & Widget::EVENT_MOUSEDOWN)) continue;
		} else {
			if (!(evmask & Widget::EVENT_MOUSEUP)) continue;
		}
		(*i).w->GetPosition(pos);
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
	return true;
}

void Container::DeleteAllChildren()
{
	for (std::list<widget_pos>::iterator i = m_children.begin(); i != m_children.end(); ++i) {
		delete (*i).w;
	}
	m_children.clear();
}

void Container::PrependChild(Widget *child, float x, float y)
{
	widget_pos wp;
	wp.w = child;
	wp.pos[0] = x; wp.pos[1] = y;
	child->SetPosition(x, y);
	child->SetParent(this);
	m_children.push_front(wp);
}
	
void Container::AppendChild(Widget *child, float x, float y)
{
	widget_pos wp;
	wp.w = child;
	wp.pos[0] = x; wp.pos[1] = y;
	child->SetPosition(x, y);
	child->SetParent(this);
	m_children.push_back(wp);
}

void Container::MoveChild(Widget *child, float x, float y)
{
	for (std::list<widget_pos>::iterator i = m_children.begin(); i != m_children.end(); ++i) {
		if ((*i).w == child) {
			(*i).pos[0] = x;
			(*i).pos[1] = y;
			child->SetPosition(x,y);
			return;
		}
	}
}

void Container::RemoveChild(Widget *child)
{
	for (std::list<widget_pos>::iterator i = m_children.begin(); i != m_children.end(); ++i) {
		if ((*i).w == child) {
			m_children.erase(i);
			return;
		}
	}
}

	
void Container::Draw()
{
	for (std::list<widget_pos>::iterator i = m_children.begin(); i != m_children.end(); ++i) {
		if (!(*i).w->IsVisible()) continue;
		glPushMatrix();
		glTranslatef((*i).pos[0], (*i).pos[1], 0);
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

void Container::ShowAll()
{
	ShowChildren();
	Show();
}

void Container::HideAll()
{
	HideChildren();
	Hide();
}

}
