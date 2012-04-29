#include "DropDown.h"
#include "Context.h"
#include "text/TextureFont.h"

namespace UI {

DropDown::DropDown(Context *context) : Widget(context), m_selected(0)
{
}

void DropDown::CalcSizePos()
{
	const float textHeight = GetContext()->GetFont()->GetHeight() + GetContext()->GetFont()->GetDescender();
	float textWidth = textHeight;
	for (std::vector<std::string>::const_iterator i = m_options.begin(); i != m_options.end(); ++i) {
		float w, h;
		GetContext()->GetFont()->MeasureString((*i).c_str(), w, h);
		if (textWidth < w) textWidth = w;
	}

	m_textPos = vector2f(Skin::s_backgroundNormal.borderWidth);
	m_textSize = vector2f(textWidth,textHeight);

	m_backgroundPos = vector2f(0);
	m_backgroundSize = m_textSize+Skin::s_backgroundNormal.borderWidth*2;

	m_buttonPos = vector2f(m_backgroundSize.x,0);
	m_buttonSize = vector2f(m_backgroundSize.y);

	m_preferredSize = vector2f(m_backgroundSize.x+m_buttonSize.x,m_backgroundSize.y);
}

vector2f DropDown::PreferredSize()
{
	CalcSizePos();
	return m_preferredSize;
}

void DropDown::Layout()
{
	if (m_preferredSize.ExactlyEqual(0))
		CalcSizePos();

	const vector2f size(GetSize());
	SetActiveArea(vector2f(std::min(m_preferredSize.x,size.x), std::min(m_preferredSize.y,size.y)));
}

void DropDown::Draw()
{
	if (IsMouseActive()) {
		GetContext()->GetSkin().DrawBackgroundActive(m_backgroundPos, m_backgroundSize);
		GetContext()->GetSkin().DrawButtonActive(m_buttonPos, m_buttonSize);
	}
	else {
		GetContext()->GetSkin().DrawBackgroundNormal(m_backgroundPos, m_backgroundSize);
		GetContext()->GetSkin().DrawButtonNormal(m_buttonPos, m_buttonSize);
	}

	if (m_selected < m_options.size())
		// XXX scissor
		GetContext()->GetFont()->RenderString(m_options[m_selected].c_str(), m_textPos.x, m_textPos.y);
}

DropDown *DropDown::AddOption(const std::string &text)
{
	m_options.push_back(text);
	return this;
}

}
