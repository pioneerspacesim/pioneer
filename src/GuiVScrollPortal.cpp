#include "libs.h"
#include "Gui.h"

namespace Gui {

VScrollPortal::VScrollPortal(float w, float h): Container()
{
	SetSize(w, h);
	m_child = 0;
	m_eventMask = EVENT_ALL;
}

void VScrollPortal::GetSizeRequested(float size[2])
{
	GetSize(size);
}

void VScrollPortal::OnChildResizeRequest(Widget *child)
{
	float size[2], rsize[2];
	GetSize(size);
	rsize[0] = size[0];
	rsize[1] = FLT_MAX;
	child->GetSizeRequested(rsize);
	rsize[0] = MIN(rsize[0], size[0]);
	m_childSizeY = rsize[1];
	child->SetSize(rsize[0], rsize[1]);
}

void VScrollPortal::Add(Widget *child)
{
	assert(m_child == 0);
	AppendChild(child, 0, 0);
	OnChildResizeRequest(child);
}

void VScrollPortal::Remove(Widget *child)
{
	assert(m_child = child);
	Container::RemoveChild(child);
	m_child = 0;
	m_childSizeY = 0;
}

void VScrollPortal::Draw()
{
	float size[2];
	GetSize(size);
	SetClipping(size[0], size[1]);

	m_scrollY = vscrollAdjust.GetValue();

	float toScroll = m_childSizeY - size[1];
	if (toScroll < 0) toScroll = 0;

	glPushMatrix();
	glTranslatef(0, -m_scrollY*toScroll, 0);
	Container::Draw();
	glPopMatrix();
	EndClipping();
}

}

