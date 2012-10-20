// Copyright © 2008-2012 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#ifndef UI_DROPDOWN_H
#define UI_DROPDOWN_H

#include "Widget.h"

namespace UI {

class List;

class DropDown : public Widget {
public:
	virtual Point PreferredSize();
	virtual void Layout();
	virtual void Draw();

	DropDown *AddOption(const std::string &text);
	const std::string &GetSelectedOption() const;
	void Clear();

	sigc::signal<void,unsigned int,const std::string &> onOptionSelected;

protected:
	friend class Context;
	DropDown(Context *context);

	void HandleClick();

private:
	void CalcSizePos();

	float m_textWidth;

	Point m_textPos, m_textSize;
	Point m_backgroundPos, m_backgroundSize;
	Point m_buttonPos, m_buttonSize;
	Point m_preferredSize;

	bool HandlePopupClick();
	void TogglePopup();

	RefCountedPtr<List> m_popup;
	bool m_popupActive;
};

}

#endif
