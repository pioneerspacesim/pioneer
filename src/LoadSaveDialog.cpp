#include "LoadSaveDialog.h"
#include "FileSelectorWidget.h"
#include "Game.h"
#include "Lang.h"
#include "Pi.h"

void LoadDialog::MainLoop()
{
	Gui::Fixed *background = new Gui::Fixed(float(Gui::Screen::GetWidth()), float(Gui::Screen::GetHeight()));
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

	Gui::Screen::AddBaseWidget(background, 0, 0);
	background->ShowAll();

	m_game = 0;

	m_done = false;
	while (!m_done)
		Gui::MainLoopIteration();
	
	Gui::Screen::RemoveBaseWidget(background);
	delete background;
}

void LoadDialog::OnClickLoad(std::string filename)
{
	if (filename.empty()) return;
	std::string fullname = join_path(GetPiSavefileDir().c_str(), filename.c_str(), 0);

	try {
		FILE *f = fopen(fullname.c_str(), "rb");
		if (!f) throw CouldNotOpenFileException();

		Serializer::Reader rd(f);
		fclose(f);

		m_game = new Game(rd);

	} catch (SavedGameCorruptException) {
		Gui::Screen::ShowBadError(Lang::GAME_LOAD_CORRUPT);

	} catch (CouldNotOpenFileException) {
		Gui::Screen::ShowBadError(Lang::GAME_LOAD_CANNOT_OPEN);
	}

	m_done = true;
}

void LoadDialog::OnClickBack()
{
	m_done = true;
}
