#include "List.h"
#include "Context.h"

namespace UI {

List::List(Context *context) : Container(context), m_selected(0)
{
	Context *c = GetContext();
	m_container = c->Background();
	m_container->SetInnerWidget(c->VBox());
	AddWidget(m_container);
}

vector2f List::PreferredSize() {
	return m_container->PreferredSize();
}

void List::Layout() {
	SetWidgetDimensions(m_container, 0, GetSize());
	m_container->Layout();
}

List *List::AddOption(const std::string &text)
{
	m_options.push_back(text);

	Context *c = GetContext();

	VBox *vbox = static_cast<VBox*>(m_container->GetInnerWidget());

	ColorBackground *background = c->ColorBackground(Color(0,0,0,0));
	vbox->PackEnd(background->SetInnerWidget(c->Label(text)));

	unsigned int index = m_optionBackgrounds.size();
	background->onMouseOver.connect(sigc::bind(sigc::mem_fun(this, &List::HandleOptionMouseOver), index));
	background->onMouseOut.connect(sigc::bind(sigc::mem_fun(this, &List::HandleOptionMouseOut), index));
	background->onClick.connect(sigc::bind(sigc::mem_fun(this, &List::HandleOptionClick), index));

	m_optionBackgrounds.push_back(background);

	return this;
}

bool List::HandleOptionMouseOver(unsigned int index)
{
	m_optionBackgrounds[index]->SetColor(Color(0,0,0,0.4f));
	return false;
}

bool List::HandleOptionMouseOut(unsigned int index)
{
	m_optionBackgrounds[index]->SetColor(Color(0,0,0, m_selected == index ? 0.6f : 0));
	return false;
}

bool List::HandleOptionClick(unsigned int index)
{
	if (m_selected != index) {
		m_optionBackgrounds[m_selected]->SetColor(Color(0,0,0,0));
		m_selected = index;
		onOptionSelected.emit(m_options[index]);
	}

	return false;
}

}
