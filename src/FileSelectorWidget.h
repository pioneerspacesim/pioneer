// Copyright © 2008-2013 Pioneer Developers. See AUTHORS.txt for details
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
	void OnClickFile(const std::string &file);

	Type m_type;
	std::string m_title;
	Gui::TextEntry *m_tentry;
};

class FileSelectorDialog {
public:
	FileSelectorDialog(FileSelectorWidget::Type type, const std::string &title);

	bool run();
	const std::string &GetFilename() const { return m_filename; }

private:
	void OnSelect(const std::string &path);
	void OnCancel();

	FileSelectorWidget *m_selector;
	std::string m_filename;
	bool m_done;
};

bool ShowFileSelectorDialog(FileSelectorWidget::Type type, const std::string &title, std::string &filename);

#endif
