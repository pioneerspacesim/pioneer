#include "Gui.h"

namespace Gui {

#define TOOLTIP_PADDING	5
#define FADE_TIME_MS	500

ToolTip::ToolTip(const char *text)
{
	m_layout = 0;
	SetText(text);
	m_createdTime = SDL_GetTicks();
}

ToolTip::ToolTip(std::string &text)
{
	m_layout = 0;
	SetText(text.c_str());
	m_createdTime = SDL_GetTicks();
}

ToolTip::~ToolTip()
{
	delete m_layout;
}

void ToolTip::CalcSize()
{
	float size[2];
	m_layout->MeasureSize(400.0, size);
	size[0] += 2*TOOLTIP_PADDING;
	SetSize(size[0], size[1]);
}

void ToolTip::SetText(const char *text)
{
	m_text = text;
	if (m_layout) delete m_layout;
	m_layout = new TextLayout(text);
	CalcSize();
}

void ToolTip::SetText(std::string &text)
{
	SetText(text.c_str());
}

void ToolTip::Draw()
{
	float size[2];
	int age = SDL_GetTicks() - m_createdTime;
	float alpha = age/float(FADE_TIME_MS); alpha = std::min(alpha, 0.75f);
	glEnable(GL_BLEND);
	GetSize(size);
	//background
	FillRect(0.f, 0.f, size[0], size[1], Color(0.2f, 0.2f, 0.6f, alpha));
	//border
	DrawRect(0.f, 0.f, size[0], size[1], Color(0.f, 0.f, 0.8f, alpha));

	glPushMatrix();
	glTranslatef(TOOLTIP_PADDING,0,0);
	glColor4f(1,1,1,alpha);
	m_layout->Render(size[0]-2*TOOLTIP_PADDING);
	glPopMatrix();
	glDisable(GL_BLEND);
}

void ToolTip::GetSizeRequested(float size[2])
{
	m_layout->MeasureSize(size[0] - 2*TOOLTIP_PADDING, size);
	size[0] += 2*TOOLTIP_PADDING;
}

}
