// Copyright © 2008-2024 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#include "LuaMetaType.h"
#include "SDL_keycode.h"
#include "buildopts.h"

#include "FileSystem.h"
#include "LuaConsole.h"
#include "LuaManager.h"
#include "LuaUtils.h"
#include "Pi.h"
#include "core/Log.h"
#include "graphics/Graphics.h"
#include "sigc++/functors/mem_fun.h"
#include "text/TextSupport.h"
#include <imgui/imgui.h>
#include <algorithm>
#include <sstream>
#include <stack>

#ifdef REMOTE_LUA_REPL
// for networking
#include <arpa/inet.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>
// end networking
#endif

#define TRUSTED_CONSOLE 1

#if TRUSTED_CONSOLE
static const char CONSOLE_CHUNK_NAME[] = "[T] console";
#else
static const char CONSOLE_CHUNK_NAME[] = "console";
#endif

static constexpr int EDIT_BUFFER_LENGTH = 1024;

LuaConsole::LuaConsole() :
	m_active(false),
	m_inputFrame(Pi::input),
	m_historyPosition(-1),
	m_precompletionStatement(),
	m_completionList()
#ifdef REMOTE_LUA_REPL
	,
	m_debugSocket(0)
#endif
{
	RegisterAutoexec();
	m_logCallbackConn = Log::GetLog()->printCallback.connect(sigc::mem_fun(this, &LuaConsole::LogCallback));
	m_editBuffer.reset(new char[EDIT_BUFFER_LENGTH]);
	std::fill_n(m_editBuffer.get(), EDIT_BUFFER_LENGTH, '\0');
}

REGISTER_INPUT_BINDING(LuaConsole)
{
	auto *group = Pi::input->GetBindingPage("General")->GetBindingGroup("Miscellaneous");
	input->AddActionBinding("BindToggleLuaConsole", group, InputBindings::Action({ SDLK_BACKQUOTE }));
}

void LuaConsole::SetupBindings()
{
	toggleLuaConsole = m_inputFrame.AddAction("BindToggleLuaConsole");
	toggleLuaConsole->onReleased.connect(sigc::mem_fun(this, &LuaConsole::Toggle));
	Pi::input->AddInputFrame(&m_inputFrame);
}

void LuaConsole::Toggle()
{
	// Don't activate the console if we're at the menu
	if (!Pi::game)
		return;

	// Force our input frame to the top by re-adding it
	m_inputFrame.modal = !m_active;
	m_active = !m_active;
	Pi::input->AddInputFrame(&m_inputFrame);
}

static int capture_traceback(lua_State *L)
{
	lua_pushstring(L, "\n");
	luaL_traceback(L, L, nullptr, 0);
	lua_concat(L, 3);
	return 1;
}

// Create the table and leave a copy on the stack for further use
static void init_global_table(lua_State *l)
{
	LUA_DEBUG_START(l);

	lua_newtable(l);
	lua_newtable(l);
	lua_pushliteral(l, "__index");
	lua_getglobal(l, "_G");
	lua_rawset(l, -3);
	lua_setmetatable(l, -2);
	lua_pushvalue(l, -1);
	lua_setfield(l, LUA_REGISTRYINDEX, "ConsoleGlobal");

	LUA_DEBUG_END(l, 1);
}

static int console_autoexec(lua_State *l)
{
	LUA_DEBUG_START(l);

	init_global_table(l); // _ENV

	LuaConsole *c = static_cast<LuaConsole *>(lua_touserdata(l, lua_upvalueindex(1)));
	RefCountedPtr<FileSystem::FileData> code = FileSystem::userFiles.ReadFile("console.lua");
	if (!code) {
		lua_pop(l, 1);
		LUA_DEBUG_END(l, 0);
		return 0;
	}

	int ret = pi_lua_loadfile(l, *code); // code, _ENV
	if (ret != LUA_OK) {
		if (ret == LUA_ERRSYNTAX) {
			const char *msg = lua_tostring(l, -1);
			Output("console.lua: %s\n", msg);
			lua_pop(l, 1);
		}
		c->AddOutput("Failed to run console.lua");
		lua_pop(l, 1); //popping _ENV
		LUA_DEBUG_END(l, 0);
		return 0;
	}

	// set the chunk's _ENV (globals) var
	lua_insert(l, -2);		  // _ENV, code
	lua_setupvalue(l, -2, 1); // code

	lua_pushcfunction(l, &capture_traceback); // traceback, code
	lua_insert(l, -2);						  // code, traceback

	ret = lua_pcall(l, 0, 0, -2);
	if (ret != LUA_OK) {
		const char *msg = lua_tostring(l, -1);
		Output("console.lua:\n%s\n", msg);
		c->AddOutput("Failed to run console.lua");
		lua_pop(l, 1);
	}

	// pop capture_traceback function
	lua_pop(l, 1);
	LUA_DEBUG_END(l, 0);

	return 0;
}

void LuaConsole::RegisterAutoexec()
{
	lua_State *L = Lua::manager->GetLuaState();
	LUA_DEBUG_START(L);
	if (!pi_lua_import(L, "Event")) {
		Output("console.lua:\nProblem when registering the autoexec script.\n");
		return;
	}
	lua_getfield(L, -1, "Register");		  // Register, Event
	lua_pushstring(L, "onGameStart");		  // "onGameStart", Register, Event
	lua_pushlightuserdata(L, this);			  // console, "onGameStart", Register, Event
	lua_pushcclosure(L, console_autoexec, 1); // autoexec, "onGameStart", Register, Event
	lua_call(L, 2, 0);						  // Event
	lua_pop(L, 1);

	LUA_DEBUG_END(L, 0);
}

void LuaConsole::LogCallback(Time::DateTime time, Log::Severity sev, std::string_view message)
{
	if (sev <= Log::Severity::Debug)
		m_outputLines.push_back(std::string(message));
}

static int callback(ImGuiInputTextCallbackData *data)
{
	static_cast<LuaConsole *>(data->UserData)->HandleCallback(data);
	return 0;
}

void LuaConsole::Draw()
{
	ImGui::SetNextWindowPos({ 0, 0 });
	ImGui::SetNextWindowSize(ImGui::GetMainViewport()->Size);
	if (ImGui::Begin("Lua Console", nullptr, ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoSavedSettings)) {
		if (ImGui::BeginChild("##TextWindow", ImVec2(0.f, -ImGui::GetFrameHeightWithSpacing() - ImGui::GetStyle().ItemSpacing.y))) {
			for (const auto &str : m_outputLines) {
				ImGui::TextUnformatted(str.c_str());
			}

			if (ImGui::GetScrollY() >= ImGui::GetScrollMaxY())
				ImGui::SetScrollHereY(1.0f);
		}
		ImGui::EndChild();
		ImGui::Separator();

		if (ImGui::IsKeyReleased(ImGuiKey_Escape))
			Toggle();

		ImGui::SetNextItemWidth(ImGui::GetColumnWidth());
		const auto inputFlags = ImGuiInputTextFlags_EnterReturnsTrue | ImGuiInputTextFlags_CallbackCompletion | ImGuiInputTextFlags_CallbackHistory;
		if (ImGui::InputText("##Console", m_editBuffer.get(), EDIT_BUFFER_LENGTH, inputFlags, callback, (void *)this)) {
			m_activeStr = std::string(m_editBuffer.get(), strlen(m_editBuffer.get()));
			if (!m_activeStr.empty()) {
				bool changed = ExecOrContinue(m_activeStr);
				if (changed)
					strncpy(m_editBuffer.get(), m_activeStr.c_str(), EDIT_BUFFER_LENGTH);
				ImGui::SetKeyboardFocusHere(-1);
			}
			m_completionList.clear();
		}
		if (ImGui::IsWindowAppearing())
			ImGui::SetKeyboardFocusHere(-1);
	}
	ImGui::End();
}

void LuaConsole::HandleCallback(ImGuiInputTextCallbackData *data)
{
	bool hasUpdate = false;

	if (data->EventFlag == ImGuiInputTextFlags_CallbackCompletion) {
		m_activeStr = std::string(data->Buf, data->BufTextLen);
		hasUpdate = OnCompletion(ImGui::GetIO().KeyShift);
	} else if (data->EventFlag == ImGuiInputTextFlags_CallbackHistory) {
		m_activeStr = std::string(data->Buf, data->BufTextLen);
		if (data->EventKey == ImGuiKey_UpArrow)
			hasUpdate = OnHistory(true);
		else if (data->EventKey == ImGuiKey_DownArrow)
			hasUpdate = OnHistory(false);
	}

	if (hasUpdate) {
		data->DeleteChars(0, data->BufTextLen);
		data->InsertChars(0, m_activeStr.c_str());
	}
}

bool LuaConsole::OnCompletion(bool backward)
{
	UpdateCompletion(m_activeStr);

	if (!m_completionList.empty()) { // We still need to test whether it failed or not.
		if (backward) {
			if (m_currentCompletion == 0)
				m_currentCompletion = m_completionList.size();
			m_currentCompletion--;
		} else {
			m_currentCompletion++;
			if (m_currentCompletion == m_completionList.size())
				m_currentCompletion = 0;
		}

		m_activeStr = m_precompletionStatement + m_completionList[m_currentCompletion];
		return true;
	}

	return false;
}

bool LuaConsole::OnHistory(bool upArrow)
{
	if (!m_statementHistory.size())
		return false;

	if (upArrow) {
		if (m_historyPosition == -1) {
			m_stashedStatement = m_activeStr;
			m_historyPosition = m_statementHistory.size() - 1;
		} else
			--m_historyPosition;
	} else {
		if (m_historyPosition == -1) {
			m_stashedStatement = m_activeStr;
		}

		++m_historyPosition;
		if (m_historyPosition >= int(m_statementHistory.size())) {
			m_historyPosition = -1;
		}
	}

	if (m_historyPosition == -1) {
		m_activeStr = m_stashedStatement;
		m_stashedStatement.clear();
	} else {
		m_activeStr = m_statementHistory[m_historyPosition];
	}

	return true;
}

void LuaConsole::UpdateCompletion(const std::string &statement)
{
	// First, split the statement into chunks.
	m_completionList.clear();
	std::stack<std::string> chunks;
	bool method = false;
	bool expect_symbolname = false;
	std::string::const_iterator current_end = statement.end();
	std::string::const_iterator current_begin = statement.begin(); // To keep record when breaking off the loop.
	for (std::string::const_reverse_iterator r_str_it = statement.rbegin();
		 r_str_it != statement.rend(); ++r_str_it) {
		if (Text::is_alphanumunderscore(*r_str_it)) {
			expect_symbolname = false;
			continue;
		} else if (expect_symbolname) // Wrong syntax.
			return;
		if (*r_str_it != '.' && (!chunks.empty() || *r_str_it != ':')) { // We are out of the expression.
			current_begin = r_str_it.base();							 // Flag the symbol marking the beginning of the expression.
			break;
		}

		expect_symbolname = true; // We hit a separator, there should be a symbol name before it.
		chunks.push(std::string(r_str_it.base(), current_end));
		if (*r_str_it == ':')				 // If it is a colon, we know chunks is empty so it is incomplete.
			method = true;					 // it must mean that we want to call a method.
		current_end = (r_str_it + 1).base(); // +1 in order to point on the CURRENT character.
	}
	if (expect_symbolname) // Again, a symbol was expected when we broke out of the loop.
		return;

	if (current_begin != current_end)
		chunks.push(std::string(current_begin, current_end));

	if (chunks.empty()) {
		return;
	}

	lua_State *l = Lua::manager->GetLuaState();
	int stackheight = lua_gettop(l);
	lua_getfield(l, LUA_REGISTRYINDEX, "ConsoleGlobal");
	// Loading the tables in which to do the name lookup
	while (chunks.size() > 1) {
		if (!lua_istable(l, -1) && !lua_isuserdata(l, -1))
			break; // Goes directly to the cleanup code anyway.
		lua_pushstring(l, chunks.top().c_str());
		lua_gettable(l, -2);
		chunks.pop();
	}

	LuaMetaTypeBase::GetNames(m_completionList, chunks.top(), method);
	if (!m_completionList.empty()) {
		std::sort(m_completionList.begin(), m_completionList.end());
		m_completionList.erase(std::unique(m_completionList.begin(), m_completionList.end()), m_completionList.end());
		// Add blank completion at the end of the list and point to it.
		m_currentCompletion = m_completionList.size();
		m_completionList.push_back("");

		m_precompletionStatement = statement;
	}
	lua_pop(l, lua_gettop(l) - stackheight); // Clean the whole stack.
}

void LuaConsole::AddOutput(const std::string &line)
{
	m_outputLines.push_back(line);
#ifdef REMOTE_LUA_REPL
	BroadcastToDebuggers(line.back() == '\n' ? line : line + '\n');
#endif
}

// Returns true if it has modified the active string, false if the current string
// should continue to be edited
bool LuaConsole::ExecOrContinue(const std::string &stmt, bool repeatStatement)
{
	int result;
	lua_State *L = Lua::manager->GetLuaState();

	// If the statement is an expression, print its final value.
	result = luaL_loadbuffer(L, ("return " + stmt).c_str(), stmt.size() + 7, CONSOLE_CHUNK_NAME);
	if (result == LUA_ERRSYNTAX)
		result = luaL_loadbuffer(L, stmt.c_str(), stmt.size(), CONSOLE_CHUNK_NAME);

	// check for an incomplete statement
	// (follows logic from the official Lua interpreter lua.c:incomplete())
	if (result == LUA_ERRSYNTAX) {
		const char eofstring[] = "<eof>";
		size_t msglen;
		const char *msg = lua_tolstring(L, -1, &msglen);
		if (msglen >= (sizeof(eofstring) - 1)) {
			const char *tail = msg + msglen - (sizeof(eofstring) - 1);
			if (strcmp(tail, eofstring) == 0) {
				// statement is incomplete -- allow the user to continue on the next line
				m_activeStr.push_back('\n');
				lua_pop(L, 1);
				return true;
			}
		}
	}

	if (result == LUA_ERRSYNTAX) {
		size_t msglen;
		const char *msg = lua_tolstring(L, -1, &msglen);
		AddOutput(std::string(msg, msglen));
		lua_pop(L, 1);
		return false;
	}

	if (result == LUA_ERRMEM) {
		// this will probably fail too, since we've apparently
		// just had a memory allocation failure...
		AddOutput("memory allocation failure");
		return false;
	}

	// set the global table
	lua_getfield(L, LUA_REGISTRYINDEX, "ConsoleGlobal");
	lua_setupvalue(L, -2, 1);

	std::istringstream stmt_stream(stmt);
	std::string string_buffer;

	if (repeatStatement) {
		std::getline(stmt_stream, string_buffer);
		AddOutput("> " + string_buffer);

		while (!stmt_stream.eof()) {
			std::getline(stmt_stream, string_buffer);
			AddOutput("  " + string_buffer);
		}
	}

	// perform a protected call
	int top = lua_gettop(L) - 1; // -1 for the chunk itself
	result = lua_pcall(L, 0, LUA_MULTRET, 0);

	if (result == LUA_ERRRUN) {
		size_t len;
		const char *s = lua_tolstring(L, -1, &len);
		AddOutput(std::string(s, len));
	} else if (result == LUA_ERRERR) {
		size_t len;
		const char *s = lua_tolstring(L, -1, &len);
		AddOutput("error in error handler: " + std::string(s, len));
	} else if (result == LUA_ERRMEM) {
		AddOutput("memory allocation failure");
	} else {
		int nresults = lua_gettop(L) - top;
		if (nresults) {
			std::ostringstream ss;

			// call tostring() on each value and display it
			lua_getglobal(L, "tostring");
			// i starts at 1 because Lua stack uses 1-based indexing
			for (int i = 1; i <= nresults; ++i) {
				ss.str(std::string());
				if (nresults > 1)
					ss << "[" << i << "] ";

				// duplicate the tostring function for the call
				lua_pushvalue(L, -1);
				lua_pushvalue(L, top + i);

				result = lua_pcall(L, 1, 1, 0);
				size_t len = 0;
				const char *s = 0;
				if (result == 0)
					s = lua_tolstring(L, -1, &len);
				ss << (s ? std::string(s, len) : "<internal error when converting result to string>");

				// pop the result
				lua_pop(L, 1);

				AddOutput(ss.str());
			}
		}
	}

	// pop all return values
	lua_settop(L, top);

	// always forget the history position and clear the stashed command
	m_historyPosition = -1;
	m_stashedStatement.clear();

	// update the history list
	if (!result) {
		// command succeeded... add it to the history unless it's just
		// an exact repeat of the immediate last command
		if (m_statementHistory.empty() || (stmt != m_statementHistory.back()))
			m_statementHistory.push_back(stmt);

		// clear the entry box
		m_activeStr.clear();
		return true;
	};

	return false;
}

/*
 * Interface: Console
 *
 * Functions to control or interact with the Lua console.
 */

/*
 * Function: AddLine
 *
 * Add a line of output to the Lua console.
 *
 * > Console.AddLine(text)
 *
 * Parameters:
 *
 *   text - the line of text to add (without a terminating newline character)
 *
 * Availability:
 *
 *   alpha 15
 *
 * Status:
 *
 *   stable
 */
static int l_console_addline(lua_State *L)
{
	if (Pi::luaConsole) {
		size_t len;
		const char *s = luaL_checklstring(L, 1, &len);
		Pi::luaConsole->AddOutput(std::string(s, len));
	}
	return 0;
}

static int l_console_print(lua_State *L)
{
	int nargs = lua_gettop(L);
	LUA_DEBUG_START(L);
	std::string line;
	lua_getglobal(L, "tostring");
	for (int i = 1; i <= nargs; ++i) {
		lua_pushvalue(L, -1);
		lua_pushvalue(L, i);
		lua_call(L, 1, 1);
		size_t len;
		const char *str = lua_tolstring(L, -1, &len);
		if (!str) {
			return luaL_error(L, "'tostring' must return a string to 'print'");
		}
		if (i > 1) {
			line += '\t';
		}
		line.append(str, len);
		lua_pop(L, 1);
	}
	lua_pop(L, 1);
	Output("%s\n", line.c_str());
	if (Pi::luaConsole) {
		Pi::luaConsole->AddOutput(line);
	}
	LUA_DEBUG_END(L, 0);
	return 0;
}

void LuaConsole::Register()
{
	lua_State *l = Lua::manager->GetLuaState();

	LUA_DEBUG_START(l);

	static const luaL_Reg methods[] = {
		{ "AddLine", l_console_addline },
		{ 0, 0 }
	};

	luaL_newlib(l, methods);
	lua_setglobal(l, "Console");

	// override the base library 'print' function
	lua_register(l, "print", &l_console_print);

	LUA_DEBUG_END(l, 0);
}

#ifdef REMOTE_LUA_REPL
void LuaConsole::OpenTCPDebugConnection(int portnumber)
{
	m_debugSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (m_debugSocket < 0) {
		Output("Error opening socket");
		return;
	}
	struct sockaddr_in destination;
	destination.sin_family = AF_INET;
	destination.sin_port = htons(portnumber);
	destination.sin_addr.s_addr = INADDR_ANY; // this should be localhost only!
	if (bind(m_debugSocket, reinterpret_cast<struct sockaddr *>(&destination), sizeof(destination)) < 0) {
		Output("Binding socket failed.\n");
		if (m_debugSocket) {
			close(m_debugSocket);
			m_debugSocket = 0;
			return;
		}
	}
	Output("Listening on TCP port %d.\n", portnumber);
	if (listen(m_debugSocket, 2) < 0) {
		Output("Listening failed.\n");
		if (m_debugSocket) {
			close(m_debugSocket);
			m_debugSocket = 0;
			return;
		}
	}
}

void LuaConsole::CloseTCPDebugConnection()
{
	const char *cmd = "CloseTCPDebugConnection";
	if (m_debugSocket) {
		errno = 0;
		if (close(m_debugSocket) == -1) {
			Output("%s: FAIL(%d).\n", cmd, errno);
		} else {
			Output("%s: OK.\n", cmd);
		}
		m_debugSocket = 0;
	} else {
		Output("%s: FAIL - already closed.\n", cmd);
	}
}

void LuaConsole::HandleTCPDebugConnections()
{
	if (m_debugSocket) {
		fd_set read_fds;
		FD_ZERO(&read_fds);
		FD_SET(m_debugSocket, &read_fds);
		struct timespec timeout = { 0, 0 };

		int res = pselect(m_debugSocket + 1, &read_fds, NULL, NULL, &timeout, NULL);
		if (res < 0 && errno != EINTR) {
			Output("pselect error %d.\n", errno);
		}
		if (FD_ISSET(m_debugSocket, &read_fds)) {
			HandleNewDebugTCPConnection(m_debugSocket);
		}
	}
	for (int sock : m_debugConnections) {
		HandleDebugTCPConnection(sock);
	}
}

void LuaConsole::HandleNewDebugTCPConnection(int socket)
{
	int sock = accept(socket, NULL, 0);
	if (sock < 0) {
		Output("Error accepting on socket.\n");
		return;
	}
	// set to non-blocking
	int status = fcntl(sock, F_SETFL, fcntl(sock, F_GETFL, 0) | O_NONBLOCK);
	if (status == -1) {
		Output("Error setting socket to non-blocking.");
	}
	// set to no buffering
	int flag = 1;
	setsockopt(sock, IPPROTO_TCP, TCP_NODELAY, reinterpret_cast<char *>(&flag), sizeof(int));

	m_debugConnections.push_back(sock);
	std::string welcome = "** Welcome to the Pioneer Remote Debugging Console!\n> ";
	BroadcastToDebuggers(welcome);
	Output("Successfully accepted connection.\n");
}

// TODO: these should not be here, do we need them generally? Maybe in utils.cpp?
static std::string &ltrim(std::string &str)
{
	auto it2 = std::find_if(str.begin(), str.end(), [](char ch) { return !std::isspace<char>(ch, std::locale::classic()); });
	str.erase(str.begin(), it2);
	return str;
}

static std::string &rtrim(std::string &str)
{
	auto it1 = std::find_if(str.rbegin(), str.rend(), [](char ch) { return !std::isspace<char>(ch, std::locale::classic()); });
	str.erase(it1.base(), str.end());
	return str;
}

static std::string &trim(std::string &str)
{
	return ltrim(rtrim(str));
}

void LuaConsole::HandleDebugTCPConnection(int sock)
{
	char buffer[4097];
	int count = read(sock, buffer, 4096);
	if (count < 0 && errno != EAGAIN) {
		Output("Error reading from socket: %d.\n", errno);
		close(sock);
		m_debugConnections.erase(std::remove(m_debugConnections.begin(), m_debugConnections.end(), sock), m_debugConnections.end());
	} else if (count > 0) {
		buffer[count] = 0;
		std::string text(buffer);
		trim(text);
		ExecOrContinue(text, false);
		BroadcastToDebuggers("\n> ");
	}
}

void LuaConsole::BroadcastToDebuggers(const std::string &message)
{
	for (int sock : m_debugConnections) {
		if (send(sock, message.c_str(), message.size(), MSG_NOSIGNAL) < 0) {
			Output("Closing debug socket, error %d.\n", errno);
			close(sock);
			m_debugConnections.erase(std::remove(m_debugConnections.begin(), m_debugConnections.end(), sock), m_debugConnections.end());
		};
	}
}
#endif
