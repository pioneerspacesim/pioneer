// Copyright © 2008-2014 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#include "List.h"
#include "Context.h"
#include <algorithm>

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

	Context *c = GetContext();

	VBox *vbox = static_cast<VBox*>(m_container->GetInnerWidget());

	int index = m_optionBackgrounds.size();

	ColorBackground *background = c->ColorBackground(Color(0,0,0, m_selected == index ? c->GetSkin().AlphaSelect_ub() : c->GetSkin().AlphaNormal_ub()));
	vbox->PackEnd(background->SetInnerWidget(c->Label(text)));

	background->onMouseOver.connect(sigc::bind(sigc::mem_fun(this, &List::HandleOptionMouseOver), index));
	background->onMouseOut.connect(sigc::bind(sigc::mem_fun(this, &List::HandleOptionMouseOut), index));
	background->onClick.connect(sigc::bind(sigc::mem_fun(this, &List::HandleOptionClick), index));

	m_optionBackgrounds.push_back(background);

	GetContext()->RequestLayout();

	return this;
}

const std::string &List::GetSelectedOption() const
{
	static const std::string empty;
	if (m_selected < 0)
		return empty;
	return m_options[m_selected];
}

bool List::SetSelectedOption(const std::string &option)
{
	std::vector<std::string>::const_iterator it = std::find(m_options.begin(), m_options.end(), option);
	if (it != m_options.end()) {
		SetSelectedIndex(it - m_options.begin());
		return true;
	} else {
		return false;
	}
}

int List::GetSelectedIndex() const
{
	return m_selected;
}

void List::SetSelectedIndex(const int index)
{
	assert(!m_options.empty() || index < 0);
	assert(index < int(m_options.size()));

	if (m_selected != index) {
		if (m_selected >= 0) {
			ColorBackground * const from = m_optionBackgrounds[m_selected];
			from->SetColor(Color(0,0,0, from->IsMouseOver()
						? GetContext()->GetSkin().AlphaHover_ub()
						: GetContext()->GetSkin().AlphaNormal_ub()));
		}

		if (index >= 0) {
			ColorBackground * const to = m_optionBackgrounds[index];
			if (!to->IsMouseOver()) {
				to->SetColor(Color(0,0,0, GetContext()->GetSkin().AlphaSelect_ub()));
			}
		}

		m_selected = index;
		onOptionSelected.emit(index, index >= 0 ? m_options[index] : "");
	}
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
	m_optionBackgrounds[index]->SetColor(Color(0,0,0, GetContext()->GetSkin().AlphaHover_ub()));
	return false;
}

bool List::HandleOptionMouseOut(int index)
{
	m_optionBackgrounds[index]->SetColor(Color(0,0,0, m_selected == index ? GetContext()->GetSkin().AlphaSelect_ub() : GetContext()->GetSkin().AlphaNormal_ub()));
	return false;
}

bool List::HandleOptionClick(int index)
{
	if ((index != m_selected) && (m_selected >= 0))
		m_optionBackgrounds[m_selected]->SetColor(Color(0,0,0, GetContext()->GetSkin().AlphaNormal_ub()));
	m_selected = index;
	onOptionSelected.emit(index, m_options[index]);

	return false;
}

}
