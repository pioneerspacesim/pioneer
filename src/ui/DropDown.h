// Copyright © 2008-2013 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#ifndef UI_DROPDOWN_H
#define UI_DROPDOWN_H

#include "Container.h"

namespace UI {

class Background;
class Label;
class Icon;
class List;

class DropDown : public Container {
public:
	virtual Point PreferredSize();
	virtual void Layout();

	DropDown *AddOption(const std::string &text);
	const std::string &GetSelectedOption() const;
	void Clear();

	sigc::signal<void,unsigned int,const std::string &> onOptionSelected;

protected:
	friend class Context;
	DropDown(Context *context);

	void HandleClick();
	void HandleMouseOver();
	void HandleMouseOut();

private:
	Background *m_container;
	Label *m_label;
	Icon *m_icon;

	bool HandlePopupClick();
	void TogglePopup();

	RefCountedPtr<List> m_popup;
	bool m_popupActive;
};

}

#endif
