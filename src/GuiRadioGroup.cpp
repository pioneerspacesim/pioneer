#include "libs.h"
#include "GuiRadioGroup.h"
#include "GuiISelectable.h"

namespace Gui {
void RadioGroup::Add(ISelectable *b)
{
	b->onSelect.connect(sigc::mem_fun(*this, &RadioGroup::OnSelected));
	m_members.push_back(b);
}

void RadioGroup::OnSelected(ISelectable *b)
{
	for(std::list<ISelectable*>::iterator i = m_members.begin(); i != m_members.end(); ++i) {
		if (*i != b) {
			(*i)->SetSelected(false);
		}
	}
}
}

