#include "MultiLineText.h"
#include "Context.h"
#include "TextLayout.h"

namespace UI {

MultiLineText::MultiLineText(Context *context, const std::string &text) : Widget(context), m_text(text)
{
	m_layout.Reset(new TextLayout(GetContext()->GetFont(), m_text));
}

vector2f MultiLineText::PreferredSize()
{
	if (!m_preferredSize.ExactlyEqual(0))
		return m_preferredSize;
	return m_layout->ComputeSize(0);
}

void MultiLineText::Layout()
{
	m_preferredSize = m_layout->ComputeSize(GetSize());
	SetActiveArea(m_preferredSize);
}

void MultiLineText::Draw()
{
	m_layout->Draw(GetSize());
}

}
