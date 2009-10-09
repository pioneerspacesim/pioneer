#include "libs.h"
#include "GenericChatForm.h"

GenericChatForm::GenericChatForm()
{
	m_msgregion = new Gui::VBox();
	m_optregion = new Gui::VBox();
       	m_msgregion->SetSpacing(5.0f);
       	m_optregion->SetSpacing(5.0f);
	Add(m_msgregion, 0, 0);
	Add(m_optregion, 0, 150);
	m_msgregion->Show();
	m_optregion->Show();
	hasOpts = false;
	Clear();
}

void GenericChatForm::Clear()
{
	m_msgregion->DeleteAllChildren();
	m_optregion->DeleteAllChildren();
	hasOpts = false;
}

void GenericChatForm::Message(const char *msg)
{
	m_msgregion->PackEnd(new Gui::Label(msg));
	ShowAll();
}

void GenericChatForm::AddOption(sigc::slot<void,GenericChatForm*,int> slot, const char *text, int val)
{
	if (!hasOpts) {
		hasOpts = true;
		m_optregion->PackStart(new Gui::Label("Suggested responses:"));
	}
	Gui::Box *box = new Gui::HBox();
	Gui::Button *b = new Gui::SolidButton();
//	b->onClick.connect(sigc::bind(sigc::mem_fun(m, &Mission::FormResponse), this, val));
	b->onClick.connect(sigc::bind(slot, this, val));
	box->SetSpacing(5.0f);
	box->PackEnd(b);
	box->PackEnd(new Gui::Label(text));
	m_optregion->PackEnd(box);
	ShowAll();
}

