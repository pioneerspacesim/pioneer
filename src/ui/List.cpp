// Copyright © 2008-2013 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#include "List.h"
#include "Context.h"

namespace UI {

List::List(Context *context) : Container(context), m_selected(-1)
{
	Context *c = GetContext();
	m_container = c->Background();
	m_container->SetInnerWidget(c->VBox());
	AddWidget(m_container);
}

Point List::PreferredSize() {
	return m_container->PreferredSize();
}

void List::Layout() {
	SetWidgetDimensions(m_container, Point(), GetSize());
	m_container->Layout();
}

List *List::AddOption(const std::string &text)
{
	m_options.push_back(text);
	if (m_selected < 0) m_selected = 0;

	Context *c = GetContext();

	VBox *vbox = static_cast<VBox*>(m_container->GetInnerWidget());

	int index = m_optionBackgrounds.size();

	ColorBackground *background = c->ColorBackground(Color(0,0,0, m_selected == index ? c->GetSkin().ListAlphaSelect() : c->GetSkin().ListAlphaNormal()));
	vbox->PackEnd(background->SetInnerWidget(c->Label(text)));

	background->onMouseOver.connect(sigc::bind(sigc::mem_fun(this, &List::HandleOptionMouseOver), index));
	background->onMouseOut.connect(sigc::bind(sigc::mem_fun(this, &List::HandleOptionMouseOut), index));
	background->onClick.connect(sigc::bind(sigc::mem_fun(this, &List::HandleOptionClick), index));

	m_optionBackgrounds.push_back(background);

	GetContext()->RequestLayout();

	return this;
}

const std::string &List::GetSelectedOption()
{
	static const std::string empty;
	if (m_selected < 0)
		return empty;
	return m_options[m_selected];
}

void List::Clear()
{
	m_options.clear();
	m_optionBackgrounds.clear();
	static_cast<VBox*>(m_container->GetInnerWidget())->Clear();
	m_selected = -1;

	GetContext()->RequestLayout();
}

bool List::HandleOptionMouseOver(int index)
{
	m_optionBackgrounds[index]->SetColor(Color(0,0,0, GetContext()->GetSkin().ListAlphaHover()));
	return false;
}

bool List::HandleOptionMouseOut(int index)
{
	m_optionBackgrounds[index]->SetColor(Color(0,0,0, m_selected == index ? GetContext()->GetSkin().ListAlphaSelect() : GetContext()->GetSkin().ListAlphaNormal()));
	return false;
}

bool List::HandleOptionClick(int index)
{
	if (m_selected != index) {
		if (m_selected >= 0)
			m_optionBackgrounds[m_selected]->SetColor(Color(0,0,0, GetContext()->GetSkin().ListAlphaNormal()));
		m_selected = index;
		onOptionSelected.emit(index, m_options[index]);
	}

	return false;
}

}
