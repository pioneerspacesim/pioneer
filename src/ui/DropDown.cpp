// Copyright © 2008-2012 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#include "DropDown.h"
#include "Context.h"
#include "ColorBackground.h"
#include "text/TextureFont.h"

namespace UI {

DropDown::DropDown(Context *context) : Widget(context), m_textWidth(0.0f), m_popupActive(false)
{
	m_popup.Reset(GetContext()->List());
	m_popup->onOptionSelected.connect(sigc::mem_fun(onOptionSelected, &sigc::signal<void,unsigned int,const std::string &>::emit));
	m_popup->onClick.connect(sigc::mem_fun(this, &DropDown::HandlePopupClick));
}

void DropDown::CalcSizePos()
{
	const float textHeight = GetContext()->GetFont(GetFont())->GetHeight() + GetContext()->GetFont(GetFont())->GetDescender();

	m_textPos = Point(GetContext()->GetSkin().BackgroundNormal().borderWidth);
	m_textSize = Point(m_textWidth,textHeight);

	m_backgroundPos = Point();
	m_backgroundSize = m_textSize + Point(GetContext()->GetSkin().BackgroundNormal().borderWidth*2);

	m_buttonPos = Point(m_backgroundSize.x,0);
	m_buttonSize = Point(m_backgroundSize.y);

	m_preferredSize = Point(m_backgroundSize.x+m_buttonSize.x,m_backgroundSize.y);
}

Point DropDown::PreferredSize()
{
	CalcSizePos();
	return m_preferredSize;
}

void DropDown::Layout()
{
	CalcSizePos();

	const Point size(GetSize());
	SetActiveArea(Point(std::min(m_preferredSize.x,size.x), std::min(m_preferredSize.y,size.y)));
}

void DropDown::Draw()
{
	if (IsMouseActive()) {
		GetContext()->GetSkin().DrawBackgroundActive(m_backgroundPos, m_backgroundSize);
		GetContext()->GetSkin().DrawButtonActive(m_buttonPos, m_buttonSize);
	}
	else {
		GetContext()->GetSkin().DrawBackgroundNormal(m_backgroundPos, m_backgroundSize);
		GetContext()->GetSkin().DrawButtonNormal(m_buttonPos, m_buttonSize);
	}

	GetContext()->GetFont(GetFont())->RenderString(m_popup->GetSelectedOption().c_str(), m_textPos.x, m_textPos.y);
}

void DropDown::HandleClick()
{
	TogglePopup();
	Widget::HandleClick();
}

bool DropDown::HandlePopupClick()
{
	TogglePopup();
	return true;
}

void DropDown::TogglePopup()
{
	Context *c = GetContext();

	if (m_popupActive) {
		c->RemoveFloatingWidget(m_popup.Get());
		m_popupActive = false;
	}

	else {
		const Point pos(GetAbsolutePosition() + Point(0, m_backgroundSize.y));
		c->AddFloatingWidget(m_popup.Get(), pos, m_popup->PreferredSize());
		m_popupActive = true;
	}

}

DropDown *DropDown::AddOption(const std::string &text)
{
	float w, h;
	GetContext()->GetFont(GetFont())->MeasureString(text.c_str(), w, h);
	if (m_textWidth < w) m_textWidth = w;

	m_popup->AddOption(text);

	return this;
}

const std::string &DropDown::GetSelectedOption() const
{
	return m_popup->GetSelectedOption();
}

void DropDown::Clear()
{
	m_popup->Clear();
	if (m_popupActive) TogglePopup();

	m_textWidth = 0.0f;
}

}
