// Copyright © 2008-2014 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#include "ChatForm.h"
#include "FormController.h"
#include "Lang.h"

ChatForm::ChatForm(FormController *controller) : FaceForm(controller), m_message(0), m_hasOptions(false), m_doSetup(true), m_close(false)
{
	Gui::VBox *outer = new Gui::VBox();
	outer->SetSpacing(10.0f);
	Add(outer, 0, 0);

	m_topRegion = new Gui::VBox();
	m_topRegion->SetSpacing(5.0f);
	outer->PackEnd(m_topRegion);

	m_bottomRegion = new Gui::VBox();
	m_bottomRegion->SetSpacing(5.0f);
	outer->PackEnd(m_bottomRegion);
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
		AddBottomWidget(new Gui::Label(Lang::SUGGESTED_RESPONSES));
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
