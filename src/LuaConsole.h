#ifndef _LUACONSOLE_H
#define _LUACONSOLE_H

#include "LuaManager.h"
#include "gui/GuiBox.h"
#include <deque>

namespace Gui {
	class Label;
	class TextEntry;
}

class TextureFont;

class LuaConsole : public Gui::VBox {
public:
	explicit LuaConsole(int displayedOutputLines);
	virtual ~LuaConsole();

	bool IsActive() const;
	Gui::TextEntry *GetTextEntryField() const { return m_entryField; }
	void AddOutput(const std::string &line);

	static void Register();
private:
	bool OnFilterKeys(const SDL_keysym*);
	void OnKeyPressed(const SDL_keysym*);
	void UpdateCompletion(const std::string & statement);
	void ExecOrContinue();

	std::deque<std::string> m_statementHistory;
	std::string m_stashedStatement;
	int m_historyPosition;
	Gui::TextEntry *m_entryField;
	std::vector<Gui::Label*> m_outputLines;
	int m_nextOutputLine;
	const int m_maxOutputLines;

	std::string m_precompletionStatement;
	std::vector<std::string> m_completionList;
	unsigned int m_currentCompletion;
};

#endif /* _LUACONSOLE_H */
