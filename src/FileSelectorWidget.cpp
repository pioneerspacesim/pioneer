// Copyright © 2008-2013 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#include "FileSelectorWidget.h"
#include "Lang.h"
#include "utils.h"
#include "Pi.h"
#include "FileSystem.h"

class SimpleLabelButton: public Gui::LabelButton
{
public:
	SimpleLabelButton(Gui::Label *label): Gui::LabelButton(label) {
		SetPadding(0.0f);
	}
	virtual void Draw() {
		m_label->Draw();
	}
};

FileSelectorWidget::FileSelectorWidget(Type type, const std::string &title) : Gui::VBox(), m_type(type), m_title(title)
{
	SetTransparency(false);
	SetSpacing(5.0f);
	SetSizeRequest(FLT_MAX, FLT_MAX);
}

void FileSelectorWidget::ShowAll()
{
	DeleteAllChildren();
	PackEnd(new Gui::Label(m_title));
	m_tentry = new Gui::TextEntry();
	PackEnd(m_tentry);

	Gui::HBox *hbox = new Gui::HBox();
	PackEnd(hbox);

	Gui::HBox *buttonBox = new Gui::HBox();
	buttonBox->SetSpacing(5.0f);
	Gui::Button *b = new Gui::LabelButton(new Gui::Label(m_type == SAVE ? Lang::SAVE : Lang::LOAD));
	b->onClick.connect(sigc::mem_fun(this, &FileSelectorWidget::OnClickAction));
	buttonBox->PackEnd(b);
	b = new Gui::LabelButton(new Gui::Label(Lang::CANCEL));
	b->onClick.connect(sigc::mem_fun(this, &FileSelectorWidget::OnClickCancel));
	buttonBox->PackEnd(b);
	PackEnd(buttonBox);

	Gui::VScrollBar *scroll = new Gui::VScrollBar();
	Gui::VScrollPortal *portal = new Gui::VScrollPortal(390);
	portal->SetTransparency(false);
	scroll->SetAdjustment(&portal->vscrollAdjust);
	hbox->PackEnd(portal);
	hbox->PackEnd(scroll);

	Gui::Box *vbox = new Gui::VBox();
	for (FileSystem::FileEnumerator files(FileSystem::userFiles, Pi::SAVE_DIR_NAME); !files.Finished(); files.Next())
	{
		const std::string name = files.Current().GetName();
		b = new SimpleLabelButton(new Gui::Label(name));
		b->onClick.connect(sigc::bind(sigc::mem_fun(this, &FileSelectorWidget::OnClickFile), name));
		vbox->PackEnd(b);
	}
	portal->Add(vbox);

	Gui::VBox::ShowAll();
}

void FileSelectorWidget::OnClickAction()
{
	onClickAction.emit(m_tentry->GetText());
}

void FileSelectorWidget::OnClickCancel() {
	onClickCancel.emit();
}

void FileSelectorWidget::OnClickFile(const std::string &file) {
	m_tentry->SetText(file);
}

FileSelectorDialog::FileSelectorDialog(FileSelectorWidget::Type type, const std::string &title)
{
	m_selector = new FileSelectorWidget(type, title);
	m_selector->onClickAction.connect(sigc::mem_fun(this, &FileSelectorDialog::OnSelect));
	m_selector->onClickCancel.connect(sigc::mem_fun(this, &FileSelectorDialog::OnCancel));
}

void FileSelectorDialog::OnSelect(const std::string &filename)
{
	if (filename.empty()) return; // ignore the action if there's no selection
	m_filename = filename;
	m_done = true;
}

void FileSelectorDialog::OnCancel()
{
	m_done = true;
}

bool FileSelectorDialog::run()
{
	// detach previous view to avoid event leakage
	View *previousView = Pi::GetView();
	Pi::SetView(0);

	Gui::Fixed *background = new Gui::Fixed(float(Gui::Screen::GetWidth()), float(Gui::Screen::GetHeight()));
	background->SetTransparency(false);
	background->SetBgColor(0,0,0,1.0);

	Gui::Fixed *outer = new Gui::Fixed(410, 410);
	outer->SetTransparency(false);
	background->Add(outer, 195, 45);

	Gui::Fixed *inner = new Gui::Fixed(400, 400);
	outer->Add(inner, 5, 5);

	inner->Add(m_selector, 0, 0);

	Gui::Screen::AddBaseWidget(background, 0, 0);
	background->ShowAll();

	m_done = false;
	m_filename.clear();
	while (!m_done)
		Gui::MainLoopIteration();

	Gui::Screen::RemoveBaseWidget(background);
	delete background;

	// restore previous view
	Pi::SetView(previousView);

	return !m_filename.empty();
}

bool ShowFileSelectorDialog(FileSelectorWidget::Type type, const std::string &title, std::string &filename)
{
	FileSelectorDialog dialog(type, title);
	bool success = dialog.run();
	if (success)
		filename = dialog.GetFilename();
	return success;
}
