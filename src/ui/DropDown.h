#ifndef _UI_DROPDOWN_H
#define _UI_DROPDOWN_H

#include "Widget.h"

namespace UI {

class List;

class DropDown : public Widget {
public:
	virtual vector2f PreferredSize();
	virtual void Layout();
	virtual void Draw();

	DropDown *AddOption(const std::string &text);
	const std::string &GetSelectedOption() const;

	sigc::signal<void,const std::string &> onOptionSelected;

protected:
	friend class Context;
	DropDown(Context *context);

	void HandleClick();

private:
	void CalcSizePos();

	float m_textWidth;

	vector2f m_textPos, m_textSize;
	vector2f m_backgroundPos, m_backgroundSize;
	vector2f m_buttonPos, m_buttonSize;
	vector2f m_preferredSize;

	bool HandlePopupClick();
	void TogglePopup();

	List *m_popup;
	bool m_popupActive;
};

}

#endif
