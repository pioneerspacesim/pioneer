// Copyright © 2008-2014 Pioneer Developers. See AUTHORS.txt for details
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
	virtual void Update();

	DropDown *AddOption(const std::string &text);
	void Clear();

	size_t NumItems() const;
	bool IsEmpty() const;

	const std::string &GetSelectedOption() const;
	bool SetSelectedOption(const std::string &option);
	int GetSelectedIndex() const;
	void SetSelectedIndex(const int index);

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

	RefCountedPtr<List> m_popup;
	bool m_popupWantToggle;
	bool m_popupActive;

	sigc::connection m_contextClickCon;
};

}

#endif
