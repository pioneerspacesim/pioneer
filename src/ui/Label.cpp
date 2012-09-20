#include "Label.h"
#include "Context.h"
#include "text/TextureFont.h"

namespace UI {

vector2f Label::PreferredSize()
{
	GetContext()->GetFont(GetFontSize())->MeasureString(m_text.c_str(), m_preferredSize.x, m_preferredSize.y);
	return m_preferredSize;
}

void Label::Layout()
{
	if (m_preferredSize.ExactlyEqual(vector2f()))
		GetContext()->GetFont(GetFontSize())->MeasureString(m_text.c_str(), m_preferredSize.x, m_preferredSize.y);

	const vector2f size = GetSize();
	SetActiveArea(vector2f(std::min(m_preferredSize.x,size.x), std::min(m_preferredSize.y,size.y)));
}

void Label::Draw()
{
	GetContext()->GetFont(GetFontSize())->RenderString(m_text.c_str(), 0.0f, 0.0f);
}

Label *Label::SetText(const std::string &text)
{
	m_text = text;
	GetContext()->RequestLayout();
	return this;
}

}
