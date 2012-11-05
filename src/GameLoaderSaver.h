// Copyright © 2008-2012 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#ifndef _GAMELOADERSAVER_H
#define _GAMELOADERSAVER_H

#include "gui/Gui.h"
#include "FileSelectorWidget.h"

class Game;

class GameLoaderSaver {
public:
	virtual void DialogMainLoop();

	const std::string &GetFilename() const { return m_filename; }

protected:
	GameLoaderSaver(FileSelectorWidget::Type type, const std::string &title);

	virtual bool OnAction() = 0;

private:
	void OnClickLoad(const std::string &filename);
	void OnClickBack();

	FileSelectorWidget::Type m_type;
	std::string m_title;

	std::string m_filename;
	bool m_done;
};

class GameLoader : public GameLoaderSaver {
public:
	GameLoader();

	Game *GetGame() const { return m_game; }

	virtual void DialogMainLoop();

	bool LoadFromFile(const std::string &filename);

protected:
	virtual bool OnAction();

private:
	Game *m_game;
};

class GameSaver : public GameLoaderSaver {
public:
	GameSaver(Game *game);

	bool SaveToFile(const std::string &filename);

protected:
	virtual bool OnAction();

private:
	Game *m_game;
};

#endif
