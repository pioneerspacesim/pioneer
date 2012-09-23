// Copyright © 2008-2012 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#ifndef _FILESELECTORWIDGET_H
#define _FILESELECTORWIDGET_H

#include "gui/Gui.h"

class FileSelectorWidget: public Gui::VBox {
public:
	enum Type { LOAD, SAVE };

	FileSelectorWidget(Type type, const std::string &title);
	void ShowAll();

	sigc::signal<void,std::string> onClickAction;
	sigc::signal<void> onClickCancel;

private:
	void OnClickAction();
	void OnClickCancel();
	void OnClickFile(std::string file);

	Type m_type;
	std::string m_title;
	Gui::TextEntry *m_tentry;
};

#endif
