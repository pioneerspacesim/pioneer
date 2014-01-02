// Copyright © 2008-2014 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#ifndef _FORMCONTROLLER_H
#define _FORMCONTROLLER_H

#include "Form.h"
#include "gui/Gui.h"

class FormController {
public:
	FormController(Gui::Stack *formStack) : m_formStack(formStack) {}

	void ActivateForm(Form *form);
	void JumpToForm(Form *form);
	void CloseForm();
	void Refresh();

	sigc::signal<void,Form*> onRefresh;
	sigc::signal<void,Form*> onClose;

private:
	Gui::Stack *m_formStack;
};

#endif
