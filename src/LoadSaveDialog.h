#ifndef _LOADSAVEDIALOG_H
#define _LOADSAVEDIALOG_H

#include "gui/Gui.h"
#include "FileSelectorWidget.h"

class Game;

class LoadSaveDialog {
public:
	virtual void MainLoop();

protected:
	LoadSaveDialog(FileSelectorWidget::Type type, const std::string &title);

	FileSelectorWidget *GetFileSelector() const { return m_fileSelector.Get(); }

	void Done() { m_done = true; }

private:
	ScopedPtr<FileSelectorWidget> m_fileSelector;
	bool m_done;
};

class LoadDialog : public LoadSaveDialog {
public:
	LoadDialog();
	Game *GetGame() const { return m_game; }

	virtual void MainLoop();

private:
	void OnClickLoad(std::string filename);
	void OnClickBack();

	Game *m_game;
};

class SaveDialog : public LoadSaveDialog {
public:
	SaveDialog(Game *game);

private:
	void OnClickLoad(std::string filename);
	void OnClickBack();

	Game *m_game;
};

#endif
