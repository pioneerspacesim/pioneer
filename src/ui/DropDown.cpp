// Copyright © 2008-2014 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#include "DropDown.h"
#include "Context.h"
#include "ColorBackground.h"
#include "text/TextureFont.h"

namespace UI {

// XXX move to config, share with TabGroup
static const Color normalColor(128, 128, 128, 255);
static const Color hoverColor(204, 204, 204, 255);
static const Color activeColor(255, 255, 255, 255);

DropDown::DropDown(Context *context) : Container(context), m_popupWantToggle(false), m_popupActive(false)
{
	Context *c = GetContext();

	m_popup.Reset(c->List());
	m_popup->onOptionSelected.connect(sigc::mem_fun(onOptionSelected, &sigc::signal<void,unsigned int,const std::string &>::emit));
	m_popup->onClick.connect(sigc::mem_fun(this, &DropDown::HandlePopupClick));

	m_container = c->Background();
	m_label = c->Label("");
	m_icon = c->Icon("ArrowDown");
	m_icon->SetColor(normalColor);
	m_container->SetInnerWidget(
		c->HBox(5)->PackEnd(
			WidgetSet(c->Expand(UI::Expand::HORIZONTAL)->SetInnerWidget(m_label), m_icon)
		)
	);

	AddWidget(m_container);
}

Point DropDown::PreferredSize()
{
	return m_container->PreferredSize();
}

void DropDown::Layout()
{
	SetWidgetDimensions(m_container, Point(), GetSize());
	m_container->Layout();
}

void DropDown::Update()
{
	if (m_popupWantToggle) {
		Context *c = GetContext();

		if (m_popupActive) {
			m_contextClickCon.disconnect();
			m_label->SetText(m_popup->GetSelectedOption());
			c->DropLayer();
			m_popupActive = false;
			m_icon->SetColor(IsMouseOver() ? hoverColor : normalColor);
		}

		else {
			const Point pos(GetAbsolutePosition() + Point(0, GetSize().y));
			m_popup->SetFont(GetFont());
			c->NewLayer()->SetInnerWidget(m_popup.Get(), pos, m_popup->PreferredSize());
			m_popupActive = true;
			m_icon->SetColor(activeColor);
			m_contextClickCon = c->onClick.connect(sigc::mem_fun(this, &DropDown::HandlePopupClick));
		}

		m_popupWantToggle = false;
	}

	Container::Update();
}

void DropDown::HandleClick()
{
	m_popupWantToggle = true;
	Widget::HandleClick();
}

bool DropDown::HandlePopupClick()
{
	m_popupWantToggle = true;
	return true;
}

void DropDown::HandleMouseOver()
{
	m_icon->SetColor(m_popupActive ? activeColor : hoverColor);
}

void DropDown::HandleMouseOut()
{
	m_icon->SetColor(m_popupActive ? activeColor : normalColor);
}

DropDown *DropDown::AddOption(const std::string &text)
{
	float w, h;
	GetContext()->GetFont(GetFont())->MeasureString(text.c_str(), w, h);

	m_popup->AddOption(text);

	m_label->SetText(m_popup->GetSelectedOption());

	return this;
}

bool DropDown::IsEmpty() const
{
	return m_popup->IsEmpty();
}

size_t DropDown::NumItems() const
{
	return m_popup->NumItems();
}

const std::string &DropDown::GetSelectedOption() const
{
	return m_popup->GetSelectedOption();
}

bool DropDown::SetSelectedOption(const std::string &option)
{
	if (m_popup->SetSelectedOption(option))
	{
		m_label->SetText(option);
		return true;
	}
	return false;
}

int DropDown::GetSelectedIndex() const
{
	return m_popup->GetSelectedIndex();
}

void DropDown::SetSelectedIndex(const int index)
{
	m_popup->SetSelectedIndex(index);
	m_label->SetText(m_popup->GetSelectedOption());
}

void DropDown::Clear()
{
	m_popup->Clear();
	if (m_popupActive)
		m_popupWantToggle = true;
}

}
