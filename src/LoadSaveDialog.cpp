#include "LoadSaveDialog.h"
#include "FileSelectorWidget.h"
#include "Game.h"
#include "Lang.h"
#include "Pi.h"

LoadSaveDialog::LoadSaveDialog(FileSelectorWidget::Type type, const std::string &title)
{
	m_fileSelector.Reset(new FileSelectorWidget(type, title));
}

void LoadSaveDialog::MainLoop()
{
	Gui::Fixed *background = new Gui::Fixed(float(Gui::Screen::GetWidth()), float(Gui::Screen::GetHeight()));
	background->SetTransparency(false);
	background->SetBgColor(0,0,0,1.0);

	Gui::Fixed *outer = new Gui::Fixed(410, 410);
	outer->SetTransparency(false);
	background->Add(outer, 195, 45);

	Gui::Fixed *inner = new Gui::Fixed(400, 400);
	outer->Add(inner, 5, 5);

	inner->Add(m_fileSelector.Get(), 0, 0);

	Gui::Screen::AddBaseWidget(background, 0, 0);
	background->ShowAll();

	m_done = false;
	while (!m_done)
		Gui::MainLoopIteration();
	
	background->Remove(m_fileSelector.Get());
	
	Gui::Screen::RemoveBaseWidget(background);
	delete background;
}


LoadDialog::LoadDialog() : LoadSaveDialog(FileSelectorWidget::LOAD, Lang::SELECT_FILENAME_TO_LOAD), m_game(0)
{
	GetFileSelector()->onClickAction.connect(sigc::mem_fun(this, &LoadDialog::OnClickLoad));
	GetFileSelector()->onClickCancel.connect(sigc::mem_fun(this, &LoadDialog::OnClickBack));
}

void LoadDialog::MainLoop()
{
	m_game = 0;
	LoadSaveDialog::MainLoop();
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

	Done();
}

void LoadDialog::OnClickBack()
{
	Done();
}


SaveDialog::SaveDialog(Game *game) : LoadSaveDialog(FileSelectorWidget::SAVE, Lang::SELECT_FILENAME_TO_SAVE), m_game(game)
{
	GetFileSelector()->onClickAction.connect(sigc::mem_fun(this, &SaveDialog::OnClickLoad));
	GetFileSelector()->onClickCancel.connect(sigc::mem_fun(this, &SaveDialog::OnClickBack));
}

void SaveDialog::OnClickLoad(std::string filename)
{
	if (filename.empty()) return;
#if 0
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

	Done();
#endif
}

void SaveDialog::OnClickBack()
{
	Done();
}
