#include "Label.h"
#include "Context.h"
#include "TextLayout.h"

namespace UI {

Label::Label(Context *context, const std::string &text) : Widget(context), m_text(text)
{
}

Metrics Label::GetMetrics(const vector2f &hint)
{
	if (!m_layout) m_layout.Reset(new TextLayout(GetContext()->GetFont(), m_text));

	vector2f want(m_layout->ComputeSize(hint));
	return Metrics(want, want, vector2f(FLT_MAX,FLT_MAX));
}

void Label::Draw()
{
	if (!m_layout) m_layout.Reset(new TextLayout(GetContext()->GetFont(), m_text));
	m_layout->Draw(GetSize());
}

}
