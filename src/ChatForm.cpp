#include "ChatForm.h"
#include "FormController.h"

ChatForm::ChatForm(FormController *controller) : FaceForm(controller), m_message(0), m_hasOptions(false), m_doSetup(true), m_close(false)
{
	m_topRegion = new Gui::VBox();
	m_topRegion->SetSpacing(5.0f);
	Add(m_topRegion, 0, 0);

	m_bottomRegion = new Gui::VBox();
	m_bottomRegion->SetSpacing(5.0f);
	Add(m_bottomRegion, 0, 0);
}

void ChatForm::ShowAll()
{
	if (m_doSetup) {
		m_doSetup = false;
		OnOptionClickedTrampoline(0);
	}
	FaceForm::ShowAll();

	float form_size[2], region_size[2];
	GetSize(form_size);
	region_size[0] = form_size[0];
	region_size[1] = form_size[1];
	m_bottomRegion->GetSizeRequested(region_size);
	MoveChild(m_bottomRegion, 0, form_size[1]-region_size[1]);
}

void ChatForm::SetMessage(std::string msg)
{
	if (msg.length() == 0) {
		if (!m_message) return;
		RemoveChild(m_message);
		delete m_message;
		m_message = 0;
		return;
	}

	if (!m_message) {
		m_message = new Gui::Label(msg);
		AddTopWidget(m_message);
		return;
	}

	m_message->SetText(msg);
}

void ChatForm::AddOption(std::string text, int val)
{
	if (!m_hasOptions) {
		AddBottomWidget(new Gui::Label("Suggested responses:"));
		m_hasOptions = true;
	}

	Gui::Box *box = new Gui::HBox();
	box->SetSpacing(5.0f);

	Gui::Button *b = new Gui::SolidButton();
	b->onClick.connect(sigc::bind(sigc::mem_fun(this, &ChatForm::OnOptionClickedTrampoline), val));

	box->PackEnd(b);
	box->PackEnd(new Gui::Label(text));

	AddBottomWidget(box);
}

void ChatForm::Clear()
{
	m_topRegion->DeleteAllChildren();
	m_bottomRegion->DeleteAllChildren();

	m_message = 0;
	m_hasOptions = false;
}

void ChatForm::Close()
{
	m_close = true;
}

void ChatForm::AddTopWidget(Gui::Widget *w)
{
	m_topRegion->PackEnd(w);
	m_topRegion->ShowAll();
}

void ChatForm::AddBottomWidget(Gui::Widget *w)
{
	m_bottomRegion->PackEnd(w);
	m_bottomRegion->ShowAll();
}

void ChatForm::OnOptionClickedTrampoline(int option)
{
	OnOptionClicked(option);
	if (m_close)
		m_formController->CloseForm();
	else
		m_formController->Refresh();
}
