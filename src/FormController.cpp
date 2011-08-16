#include "FormController.h"

void FormController::ActivateForm(Form *form)
{
	m_formStack->Push(form);
	Refresh();
}

void FormController::JumpToForm(Form *form)
{
	if (m_formStack->Size() > 0)
		onClose.emit(static_cast<Form*>(m_formStack->Top()));
	m_formStack->JumpTo(form);
	Refresh();
}

void FormController::CloseForm()
{
	if (m_formStack->Size() > 0)
		onClose.emit(static_cast<Form*>(m_formStack->Top()));
	m_formStack->Pop();
	Refresh();
}

void FormController::Refresh()
{
	if (m_formStack->Size() == 0) return;
	onRefresh.emit(static_cast<Form*>(m_formStack->Top()));
}
