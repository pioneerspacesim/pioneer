#ifndef _LOADSAVEDIALOG_H
#define _LOADSAVEDIALOG_H

#include "gui/Gui.h"

class Game;

class LoadDialog {
public:
	void MainLoop();

	Game *GetGame() { return m_game; }

private:
	void OnClickLoad(std::string filename);
	void OnClickBack();

	bool m_done;
	Game *m_game;
};

#endif
