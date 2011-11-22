#include "LoadSaveDialog.h"
#include "FileSelectorWidget.h"
#include "Game.h"
#include "Lang.h"
#include "Pi.h"

LoadSaveDialog::LoadSaveDialog(FileSelectorWidget::Type type, const std::string &title) : m_type(type), m_title(title)
{
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

	FileSelectorWidget *fileSelector = new FileSelectorWidget(m_type, m_title);
	inner->Add(fileSelector, 0, 0);

	fileSelector->onClickAction.connect(sigc::mem_fun(this, &LoadSaveDialog::OnClickLoad));
	fileSelector->onClickCancel.connect(sigc::mem_fun(this, &LoadSaveDialog::OnClickBack));

	Gui::Screen::AddBaseWidget(background, 0, 0);
	background->ShowAll();

	m_done = false;
	while (!m_done)
		Gui::MainLoopIteration();
	
	Gui::Screen::RemoveBaseWidget(background);
	delete background;
}

void LoadSaveDialog::OnClickLoad(std::string filename)
{
	if (filename.empty()) return;
	m_filename = join_path(GetPiSavefileDir().c_str(), filename.c_str(), 0);
	if (!OnAction())
		m_filename = "";
	m_done = true;
}

void LoadSaveDialog::OnClickBack()
{
	m_done = true;
}


LoadDialog::LoadDialog() : LoadSaveDialog(FileSelectorWidget::LOAD, Lang::SELECT_FILENAME_TO_LOAD), m_game(0)
{
}

void LoadDialog::MainLoop()
{
	m_game = 0;
	LoadSaveDialog::MainLoop();
}

bool LoadDialog::OnAction()
{
	try {
		FILE *f = fopen(GetFilename().c_str(), "rb");
		if (!f) throw CouldNotOpenFileException();

		Serializer::Reader rd(f);
		fclose(f);

		m_game = new Game(rd);
	}
	catch (SavedGameCorruptException) {
		Gui::Screen::ShowBadError(Lang::GAME_LOAD_CORRUPT);
	}
	catch (CouldNotOpenFileException) {
		Gui::Screen::ShowBadError(Lang::GAME_LOAD_CANNOT_OPEN);
	}

	return m_game != 0;
}


SaveDialog::SaveDialog(Game *game) : LoadSaveDialog(FileSelectorWidget::SAVE, Lang::SELECT_FILENAME_TO_SAVE), m_game(game)
{
}

bool SaveDialog::OnAction()
{
	bool success = false;
	try {
		Serializer::Writer wr;
		m_game->Serialize(wr);

		const std::string data = wr.GetData();

		FILE *f = fopen(GetFilename().c_str(), "wb");
		if (!f) throw CouldNotOpenFileException();

		size_t nwritten = fwrite(data.data(), data.length(), 1, f);
		fclose(f);

		if (nwritten != 1) throw CouldNotWriteToFileException();

		success = true;
	}
	catch (CouldNotOpenFileException) {
		Gui::Screen::ShowBadError(Lang::GAME_LOAD_CANNOT_OPEN);
	}
	catch (CouldNotWriteToFileException) {
		Gui::Screen::ShowBadError(Lang::GAME_SAVE_CANNOT_WRITE);
	}

	return success;
}
