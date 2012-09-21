#ifndef _UI_LIST_H
#define _UI_LIST_H

#include "Container.h"

namespace UI {

class Background;
class ColorBackground;

class List : public Container {
public:
	virtual Point PreferredSize();
	virtual void Layout();

	List *AddOption(const std::string &text);
	const std::string &GetSelectedOption();
	void Clear();

	sigc::signal<void,unsigned int,const std::string &> onOptionSelected;

protected:
	friend class Context;
	List(Context *context);

private:
	std::vector<std::string> m_options;
	int m_selected;

	Background *m_container;
	std::vector<ColorBackground*> m_optionBackgrounds;

	bool HandleOptionMouseOver(int index);
	bool HandleOptionMouseOut(int index);
	bool HandleOptionClick(int index);
};

}

#endif
