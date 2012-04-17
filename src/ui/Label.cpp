#include "Label.h"
#include "Context.h"
#include "TextLayout.h"

namespace UI {

Label::Label(Context *context, const std::string &text) : Widget(context), m_text(text)
{
	m_layout.Reset(new TextLayout(GetContext()->GetFont(), m_text));
}

Metrics Label::GetMetrics(const vector2f &hint)
{
	vector2f want(m_layout->ComputeSize(hint));
	return Metrics(want, want, vector2f(FLT_MAX,FLT_MAX));
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
