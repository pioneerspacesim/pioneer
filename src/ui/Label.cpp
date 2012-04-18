#include "Label.h"
#include "Context.h"
#include "TextLayout.h"

namespace UI {

Label::Label(Context *context, const std::string &text) : Widget(context), m_text(text)
{
	m_layout.Reset(new TextLayout(GetContext()->GetFont(), m_text));
}

vector2f Label::PreferredSize()
{
	return m_layout->ComputeSize(0);
}

void Label::Layout()
{
	SetActiveArea(m_layout->ComputeSize(GetSize()));
}

void Label::Draw()
{
	m_layout->Draw(GetSize());
}

}
