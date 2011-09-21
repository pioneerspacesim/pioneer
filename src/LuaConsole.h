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
	void ExecOrContinue();

	std::deque<std::string> m_statementHistory;
	std::string m_stashedStatement;
	int m_historyPosition;
	Gui::TextEntry *m_entryField;
	std::vector<Gui::Label*> m_outputLines;
	int m_nextOutputLine;
	const int m_maxOutputLines;
};

#endif /* _LUACHATFORM_H */
