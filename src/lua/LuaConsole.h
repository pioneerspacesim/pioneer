// Copyright © 2008-2024 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#ifndef _LUACONSOLE_H
#define _LUACONSOLE_H

#include "Input.h"
#include "LuaManager.h"
#include "RefCounted.h"
#include "ConnectionTicket.h"
#include "core/Log.h"
#include <deque>

struct ImGuiInputTextCallbackData;

class LuaConsole {
public:
	LuaConsole();

	void SetupBindings();
	void Toggle();

	bool IsActive() const { return m_active; }
	void AddOutput(const std::string &line);

#ifdef REMOTE_LUA_REPL
	void OpenTCPDebugConnection(int portnumber);
	void CloseTCPDebugConnection();
	void HandleTCPDebugConnections();
#endif

	static void Register();
	void HandleCallback(ImGuiInputTextCallbackData *data);
	void Draw();

private:
	bool OnCompletion(bool backward);
	bool OnHistory(bool upArrow);
	void LogCallback(Time::DateTime, Log::Severity, std::string_view);

	bool ExecOrContinue(const std::string &stmt, bool repeatStatement = true);
	void UpdateCompletion(const std::string &statement);
	void RegisterAutoexec();

#ifdef REMOTE_LUA_REPL
	void HandleNewDebugTCPConnection(int socket);
	void HandleDebugTCPConnection(int socket);
	void BroadcastToDebuggers(const std::string &message);
#endif

	bool m_active;
	Input::InputFrame m_inputFrame;
	InputBindings::Action *toggleLuaConsole;
	ConnectionTicket m_logCallbackConn;

	// Output log
	std::vector<std::string> m_outputLines;

	// statement history
	std::deque<std::string> m_statementHistory;
	std::string m_stashedStatement;
	std::string m_activeStr;
	std::unique_ptr<char[]> m_editBuffer;
	int m_historyPosition;

	// autocompletion
	std::string m_precompletionStatement;
	std::vector<std::string> m_completionList;
	unsigned int m_currentCompletion;

#ifdef REMOTE_LUA_REPL
	int m_debugSocket;
	std::vector<int> m_debugConnections;
#endif
};

#endif /* _LUACONSOLE_H */
