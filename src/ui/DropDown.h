#ifndef _UI_DROPDOWN_H
#define _UI_DROPDOWN_H

#include "Widget.h"

namespace UI {

class Context;
class ColorBackground;

class DropDown : public Widget {
public:
	virtual vector2f PreferredSize();
	virtual void Layout();
	virtual void Draw();

	DropDown *AddOption(const std::string &text);

protected:
	friend class Context;
	DropDown(Context *context);

	void HandleClick();

private:
	void CalcSizePos();

	vector2f m_textPos, m_textSize;
	vector2f m_backgroundPos, m_backgroundSize;
	vector2f m_buttonPos, m_buttonSize;
	vector2f m_preferredSize;

	std::vector<std::string> m_options;
	unsigned int m_selected;

	void BuildPopup();

	Widget *m_popup;
	bool m_popupActive;

	std::vector<ColorBackground*> m_backgrounds;

	bool HandlePopupOptionMouseOver(UI::ColorBackground *background);
	bool HandlePopupOptionMouseOut(UI::ColorBackground *background);
};

}

#endif
