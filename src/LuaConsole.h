// Copyright © 2008-2018 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#ifndef _LUACONSOLE_H
#define _LUACONSOLE_H

#include "LuaManager.h"
#include "ui/Widget.h"
#include "RefCounted.h"
#include <deque>

namespace UI {
	class TextEntry;
	class MultiLineText;
	class Scroller;
}

class LuaConsole {
public:
	LuaConsole();
	virtual ~LuaConsole();

	void Toggle();

	bool IsActive() const { return m_active; }
	void AddOutput(const std::string &line);

#ifdef REMOTE_LUA_REPL
	void OpenTCPDebugConnection(int portnumber);
	void HandleTCPDebugConnections();
#endif

	static void Register();
private:
	bool OnKeyDown(const UI::KeyboardEvent &event);
	void OnChange(const std::string &text);
	void OnEnter(const std::string &text);

	void ExecOrContinue(const std::string &stmt, bool repeatStatement=true);

	void OnKeyPressed(const SDL_Keysym*);
	void OnTextChanged();
	void UpdateCompletion(const std::string & statement);
	void RegisterAutoexec();

#ifdef REMOTE_LUA_REPL
	void HandleNewDebugTCPConnection(int socket);
	void HandleDebugTCPConnection(int socket);
	void BroadcastToDebuggers(const std::string &message);
#endif

	bool m_active;

	RefCountedPtr<UI::Widget> m_container;
	UI::MultiLineText *m_output;
	UI::TextEntry *m_entry;
	UI::Scroller *m_scroller;

	std::deque<std::string> m_statementHistory;
	std::string m_stashedStatement;
	int m_historyPosition;
	int m_nextOutputLine;

	std::string m_precompletionStatement;
	std::vector<std::string> m_completionList;
	unsigned int m_currentCompletion;

#ifdef REMOTE_LUA_REPL
	int m_debugSocket;
	std::vector<int> m_debugConnections;
#endif
};

#endif /* _LUACONSOLE_H */
