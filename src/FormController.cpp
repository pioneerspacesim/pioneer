#include "FormController.h"

void FormController::ActivateForm(Form *form)
{
	m_formStack.push(form);
	Refresh();
}

void FormController::JumpToForm(Form *form)
{
	while (m_formStack.size()) {
		Form *old_form = static_cast<Form*>(m_formStack.top());
		old_form->OnClose();
		onClose.emit(old_form);
		m_formStack.pop();
	}
	ActivateForm(form);
}

void FormController::CloseForm()
{
	if (m_formStack.size()) {
		Form *form = static_cast<Form*>(m_formStack.top());
		form->OnClose();
		onClose.emit(form);
		m_formStack.pop();
	}
	m_formStack.pop();
	Refresh();
}

void FormController::Refresh()
{
	if (m_formStack.size() == 0) return;
	onRefresh.emit(static_cast<Form*>(m_formStack.top()));
}
