// Copyright © 2008-2014 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#ifndef _LUACONSOLE_H
#define _LUACONSOLE_H

#include "LuaManager.h"
#include "ui/Widget.h"
#include "ui/TextEntry.h"
#include "ui/MultiLineText.h"
#include "RefCounted.h"
#include <deque>

class LuaConsole {
public:
	explicit LuaConsole(int displayedOutputLines);
	virtual ~LuaConsole();

	void Toggle();

	bool IsActive() const { return m_active; }
	void AddOutput(const std::string &line);

	static void Register();
private:
	bool OnKeyDown(const UI::KeyboardEvent &event);
	void OnChange(const std::string &text);
	void OnEnter(const std::string &text);

	void ExecOrContinue(const std::string &stmt);

	void OnKeyPressed(const SDL_Keysym*);
	void OnTextChanged();
	void UpdateCompletion(const std::string & statement);
	void RunAutoexec();

	bool m_active;

	std::deque<std::string> m_statementHistory;
	std::string m_stashedStatement;
	int m_historyPosition;
	RefCountedPtr<UI::Widget> m_container;
	UI::MultiLineText *m_output;
	UI::TextEntry *m_entry;
	int m_nextOutputLine;
	const int m_maxOutputLines;

	std::string m_precompletionStatement;
	std::vector<std::string> m_completionList;
	unsigned int m_currentCompletion;
};

#endif /* _LUACONSOLE_H */
