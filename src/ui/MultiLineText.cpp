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
	return m_layout->ComputeSize(0);
}

void MultiLineText::Layout()
{
	SetActiveArea(m_layout->ComputeSize(GetSize()));
}

void MultiLineText::Draw()
{
	m_layout->Draw(GetSize());
}

}
