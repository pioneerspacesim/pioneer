#include "ChatForm.h"

ChatForm::ChatForm() : FaceForm()
{
	m_message = new Gui::Label("");
	Add(m_message, 10, 10);

	m_options = new Gui::VBox();
	m_options->SetSpacing(5.0f);
	Add(m_options, 0, 160);

	Clear();
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

	ShowAll();
}

void ChatForm::Clear()
{
	m_message->SetText("");

	m_options->DeleteAllChildren();

	m_hasOptions = false;
}
