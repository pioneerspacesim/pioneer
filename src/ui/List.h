// Copyright © 2008-2014 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#ifndef UI_LIST_H
#define UI_LIST_H

#include "Container.h"

namespace UI {

class Background;
class ColorBackground;

class List : public Container {
public:
	virtual Point PreferredSize();
	virtual void Layout();

	List *AddOption(const std::string &text);
	void Clear();

	size_t NumItems() const { return m_options.size(); }
	bool IsEmpty() const { return m_options.empty(); }

	const std::string &GetSelectedOption() const;
	bool SetSelectedOption(const std::string &option);
	int GetSelectedIndex() const;
	void SetSelectedIndex(const int index);

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
