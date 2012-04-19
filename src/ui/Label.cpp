#include "Label.h"
#include "Context.h"
#include "text/TextureFont.h"

namespace UI {

vector2f Label::PreferredSize()
{
	GetContext()->GetFont()->MeasureString(m_text.c_str(), m_preferredSize.x, m_preferredSize.y);
	return m_preferredSize;
}

void Label::Layout()
{
	const vector2f size = GetSize();
	SetActiveArea(vector2f(std::min(m_preferredSize.x,size.x), std::min(m_preferredSize.y,size.y)));
}

void Label::Draw()
{
	GetContext()->GetFont()->RenderString(m_text.c_str(), 0.0f, 0.0f);
}

}
