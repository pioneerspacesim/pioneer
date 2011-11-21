#include "LoadSaveDialog.h"
#include "gui/Gui.h"
#include "FileSelectorWidget.h"
#include "Lang.h"
#include "Pi.h"

LoadDialog::LoadDialog() : m_done(false)
{
	Gui::Fixed *background = new Gui::Fixed(float(Gui::Screen::GetWidth()), float(Gui::Screen::GetHeight()));
	Gui::Screen::AddBaseWidget(background, 0, 0);
	background->SetTransparency(false);
	background->SetBgColor(0,0,0,1.0);

	Gui::Fixed *outer = new Gui::Fixed(410, 410);
	outer->SetTransparency(false);
	background->Add(outer, 195, 45);

	Gui::Fixed *inner = new Gui::Fixed(400, 400);
	outer->Add(inner, 5, 5);

	FileSelectorWidget *fileSelector = new FileSelectorWidget(FileSelectorWidget::LOAD, Lang::SELECT_FILENAME_TO_LOAD);
	inner->Add(fileSelector, 0, 0);

	fileSelector->onClickAction.connect(sigc::mem_fun(this, &LoadDialog::OnClickLoad));
	fileSelector->onClickCancel.connect(sigc::mem_fun(this, &LoadDialog::OnClickBack));

	background->ShowAll();
}

void LoadDialog::MainLoop()
{
	while (!m_done)
		Gui::MainLoopIteration();
}

void LoadDialog::OnClickLoad(std::string filename)
{
}

void LoadDialog::OnClickBack()
{
}
