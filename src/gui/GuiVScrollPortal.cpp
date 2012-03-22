#include "libs.h"
#include "Gui.h"
#include "vector2.h"

#define MINIMUM_HEIGHT (100.0f)

namespace Gui {

VScrollPortal::VScrollPortal(float forceWidth):
	Container(),
	m_forceWidth(forceWidth),
	m_child(0)
{
	m_eventMask = EVENT_ALL;
}

VScrollPortal::VScrollPortal():
	Container(),
	m_forceWidth(0),
	m_child(0)
{
	m_eventMask = EVENT_ALL;
}

void VScrollPortal::GetSizeRequested(float size[2])
{
	if (m_child)
		m_child->GetSizeRequested(size);
	else
		size[0] = size[1] = 0;

	if (m_forceWidth > 0.0f) size[0] = m_forceWidth;
}

void VScrollPortal::GetMinimumSize(float size[2])
{
	if (m_child) {
		m_child->GetSizeRequested(size);
		size[1] = std::min(size[1], MINIMUM_HEIGHT);
	}
	else
		size[0] = size[1] = 0;

	if (m_forceWidth > 0.0f) size[0] = m_forceWidth;
}

void VScrollPortal::OnChildResizeRequest(Widget *child)
{
	assert(child == m_child);
	float size[2], rsize[2];
	GetSize(size);
	rsize[0] = size[0];
	rsize[1] = FLT_MAX;
	child->GetSizeRequested(rsize);
	rsize[0] = std::min(rsize[0], size[0]);
	m_childSizeY = rsize[1];
	child->SetSize(rsize[0], rsize[1]);
}

void VScrollPortal::UpdateAllChildSizes()
{
	if (m_child) OnChildResizeRequest(m_child);
}

void VScrollPortal::Add(Widget *child)
{
	assert(m_child == 0);
	m_child = child;
	AppendChild(child, 0, 0);
	OnChildResizeRequest(child);
}

void VScrollPortal::Remove(Widget *child)
{
	assert(m_child == child);
	Container::RemoveChild(child);
	m_child = 0;
	m_childSizeY = 0;
}

float VScrollPortal::GetScrollPixels()
{
	float size[2];
	GetSize(size);
	return m_scrollY*((m_childSizeY-size[1]) > 0 ? (m_childSizeY-size[1]) : 0);
}

bool VScrollPortal::OnMouseDown(MouseButtonEvent *e)
{
	if (e->button == 4 || e->button == 5) {
		float change = e->button == 4 ? -0.1 : 0.1;
		float pos = vscrollAdjust.GetValue();
		vscrollAdjust.SetValue(Clamp(pos+change, 0.0f, 1.0f));
		return false;
	}

	e->y += GetScrollPixels();
	return Container::OnMouseDown(e);
}
bool VScrollPortal::OnMouseUp(MouseButtonEvent *e)
{
	e->y += GetScrollPixels();
	return Container::OnMouseUp(e);
}
bool VScrollPortal::OnMouseMotion(MouseMotionEvent *e)
{
	e->y += GetScrollPixels();
	return Container::OnMouseMotion(e);
}

void VScrollPortal::Draw()
{
	SetScissor(true);

	float size[2];
	GetSize(size);

	m_scrollY = vscrollAdjust.GetValue();

	float toScroll = m_childSizeY - size[1];
	if (toScroll < 0) toScroll = 0;
	
	float scale[2];
	Screen::GetCoords2Pixels(scale);

	glPushMatrix();
	// scroll to whole pixel locations whatever the resolution
	glTranslatef(0, floor((-m_scrollY*toScroll)/scale[1])*scale[1], 0);
	Container::Draw();
	glPopMatrix();

	SetScissor(false);
}

}

