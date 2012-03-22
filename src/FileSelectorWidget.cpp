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
	for (FileSystem::FileEnumerator files(FileSystem::rawFileSystem, Pi::GetSaveDir()); !files.Finished(); files.Next())
	{
		std::string name = files.Current().GetName();
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

void FileSelectorWidget::OnClickFile(std::string file) {
	m_tentry->SetText(file);
}
