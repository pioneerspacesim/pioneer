#ifndef _FORMCONTROLLER_H
#define _FORMCONTROLLER_H

#include "Form.h"
#include <stack>

class FormController {
public:
	void ActivateForm(Form *form);
	void JumpToForm(Form *form);
	void CloseForm();
	void Refresh();

	sigc::signal<void,Form*> onRefresh;
	sigc::signal<void,Form*> onClose;

private:
	std::stack<Form*> m_formStack;
};

#endif
