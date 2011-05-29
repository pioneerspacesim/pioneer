#include "ChatForm.h"
#include "FormController.h"

ChatForm::ChatForm(FormController *controller) : FaceForm(controller), m_hasOptions(false), m_doSetup(true), m_close(false)
{
	m_message = new Gui::Label("");
	Add(m_message, 0, 0);

	m_options = new Gui::VBox();
	m_options->SetSpacing(5.0f);
	Add(m_options, 0, 160);
}

void ChatForm::ShowAll()
{
	if (m_doSetup) {
		m_doSetup = false;
		OnOptionClickedTrampoline(0);
	}
	FaceForm::ShowAll();
}

void ChatForm::SetMessage(std::string msg)
{
	m_message->SetText(msg);
}

void ChatForm::AddOption(std::string text, int val)
{
	if (!m_hasOptions) {
		m_options->PackStart(new Gui::Label("Suggested responses:"));
		m_hasOptions = true;
	}

	Gui::Box *box = new Gui::HBox();
	box->SetSpacing(5.0f);

	Gui::Button *b = new Gui::SolidButton();
	b->onClick.connect(sigc::bind(sigc::mem_fun(this, &ChatForm::OnOptionClickedTrampoline), val));

	box->PackEnd(b);
	box->PackEnd(new Gui::Label(text));

	m_options->PackEnd(box);

	m_options->ShowAll();
}

void ChatForm::Clear()
{
	m_message->SetText("");

	m_options->DeleteAllChildren();

	m_hasOptions = false;
}

void ChatForm::Close()
{
	m_close = true;
}

void ChatForm::OnOptionClickedTrampoline(int option)
{
	OnOptionClicked(option);
	if (m_close)
		m_formController->CloseForm();
	else
		m_formController->Refresh();
}
