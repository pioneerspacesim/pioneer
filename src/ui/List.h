#ifndef _UI_LIST_H
#define _UI_LIST_H

#include "Container.h"

namespace UI {

class Background;
class ColorBackground;

class List : public Container {
public:
	virtual vector2f PreferredSize();
	virtual void Layout();

	virtual void RequestResize();

	List *AddOption(const std::string &text);
	const std::string &GetSelectedOption() const { return m_options[m_selected]; }

	sigc::signal<void,unsigned int,const std::string &> onOptionSelected;

protected:
	friend class Context;
	List(Context *context);

private:
	std::vector<std::string> m_options;
	unsigned int m_selected;

	Background *m_container;
	std::vector<ColorBackground*> m_optionBackgrounds;

	bool HandleOptionMouseOver(unsigned int index);
	bool HandleOptionMouseOut(unsigned int index);
	bool HandleOptionClick(unsigned int index);
};

}

#endif
