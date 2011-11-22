#ifndef _LOADSAVEDIALOG_H
#define _LOADSAVEDIALOG_H

#include "gui/Gui.h"
#include "FileSelectorWidget.h"

class Game;

class LoadSaveDialog {
public:
	virtual void MainLoop();

	const std::string &GetFilename() const { return m_filename; }

protected:
	LoadSaveDialog(FileSelectorWidget::Type type, const std::string &title);

	virtual bool OnAction() = 0;

private:
	void OnClickLoad(std::string filename);
	void OnClickBack();

	FileSelectorWidget::Type m_type;
	std::string m_title;

	std::string m_filename;
	bool m_done;
};

class LoadDialog : public LoadSaveDialog {
public:
	LoadDialog();
	Game *GetGame() const { return m_game; }

	virtual void MainLoop();

protected:
	virtual bool OnAction();

private:
	Game *m_game;
};

class SaveDialog : public LoadSaveDialog {
public:
	SaveDialog(Game *game);

protected:
	virtual bool OnAction();

private:
	Game *m_game;
};

#endif
