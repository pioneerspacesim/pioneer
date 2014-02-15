// Copyright © 2008-2014 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#include "LuaConsole.h"
#include "LuaManager.h"
#include "Pi.h"
#include "ui/Context.h"
#include "text/TextureFont.h"
#include "text/TextSupport.h"
#include "KeyBindings.h"
#include "FileSystem.h"
#include "LuaUtils.h"
#include <sstream>
#include <stack>
#include <algorithm>

#define TRUSTED_CONSOLE 1

#if TRUSTED_CONSOLE
static const char CONSOLE_CHUNK_NAME[] = "[T] console";
#else
static const char CONSOLE_CHUNK_NAME[] = "console";
#endif

LuaConsole::LuaConsole():
	m_active(false),
	m_precompletionStatement(),
	m_completionList() {

	m_output = Pi::ui->MultiLineText("");
	m_entry = Pi::ui->TextEntry();

	m_scroller = Pi::ui->Scroller()->SetInnerWidget(m_output);

	m_container.Reset(Pi::ui->Margin(10)->SetInnerWidget(
		Pi::ui->ColorBackground(Color(0,0,0,0xc0))->SetInnerWidget(
			Pi::ui->VBox()->PackEnd(UI::WidgetSet(
				Pi::ui->Expand()->SetInnerWidget(
					m_scroller
				),
				m_entry
			))
		)
	));

	m_container->SetFont(UI::Widget::FONT_MONO_NORMAL);

	m_entry->onKeyDown.connect(sigc::mem_fun(this, &LuaConsole::OnKeyDown));
	m_entry->onChange.connect(sigc::mem_fun(this, &LuaConsole::OnChange));
	m_entry->onEnter.connect(sigc::mem_fun(this, &LuaConsole::OnEnter));

	m_historyPosition = -1;

	RegisterAutoexec();
}

void LuaConsole::Toggle()
{
	if (m_active)
		Pi::ui->DropLayer();
	else {
		Pi::ui->NewLayer()->SetInnerWidget(m_container.Get());
		Pi::ui->SelectWidget(m_entry);
	}
	m_active = !m_active;
}

static int capture_traceback(lua_State *L) {
	lua_pushstring(L, "\n");
	luaL_traceback(L, L, nullptr, 0);
	lua_concat(L, 3);
	return 1;
}

// Create the table and leave a copy on the stack for further use
static void init_global_table(lua_State *l) {
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

static int console_autoexec(lua_State* l) {
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
	lua_insert(l, -2); // _ENV, code
	lua_setupvalue(l, -2, 1); // code

	lua_pushcfunction(l, &capture_traceback); // traceback, code
	lua_insert(l, -2); // code, traceback

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

void LuaConsole::RegisterAutoexec() {
	lua_State *L = Lua::manager->GetLuaState();
	LUA_DEBUG_START(L);
	if (!pi_lua_import(L, "Event")) {
		Output("console.lua:\nProblem when registering the autoexec script.\n");
		return;
	}
	lua_getfield(L, -1, "Register"); // Register, Event
	lua_pushstring(L, "onGameStart"); // "onGameStart", Register, Event
	lua_pushlightuserdata(L, this); // console, "onGameStart", Register, Event
	lua_pushcclosure(L, console_autoexec, 1); // autoexec, "onGameStart", Register, Event
	lua_call(L, 2, 0); // Event
	lua_pop(L, 1);

	LUA_DEBUG_END(L, 0);
}

LuaConsole::~LuaConsole() {}

bool LuaConsole::OnKeyDown(const UI::KeyboardEvent &event) {

	switch (event.keysym.sym) {
		case SDLK_UP:
		case SDLK_DOWN: {
			if (m_historyPosition == -1) {
				if (event.keysym.sym == SDLK_UP) {
					m_historyPosition = (m_statementHistory.size() - 1);
					if (m_historyPosition != -1) {
						m_stashedStatement = m_entry->GetText();
						m_entry->SetText(m_statementHistory[m_historyPosition]);
					}
				}
			} else {
				if (event.keysym.sym == SDLK_DOWN) {
					++m_historyPosition;
					if (m_historyPosition >= int(m_statementHistory.size())) {
						m_historyPosition = -1;
						m_entry->SetText(m_stashedStatement);
						m_stashedStatement.clear();
					} else {
						m_entry->SetText(m_statementHistory[m_historyPosition]);
					}
				} else {
					if (m_historyPosition > 0) {
						--m_historyPosition;
						m_entry->SetText(m_statementHistory[m_historyPosition]);
					}
				}
			}

			return true;
		}

		case SDLK_u:
		case SDLK_w:
			if (event.keysym.mod & KMOD_CTRL) {
				// TextEntry already cleared the input, we must cleanup the history
				m_stashedStatement.clear();
				m_historyPosition = -1;
				return true;
			}
			break;

		case SDLK_l:
			if (event.keysym.mod & KMOD_CTRL) {
				m_output->SetText("");
				return true;
			}
			break;

		case SDLK_TAB:
			if (m_completionList.empty()) {
				UpdateCompletion(m_entry->GetText());
			}
			if (!m_completionList.empty()) { // We still need to test whether it failed or not.
				if (event.keysym.mod & KMOD_SHIFT) {
					if (m_currentCompletion == 0)
						m_currentCompletion = m_completionList.size();
					m_currentCompletion--;
				} else {
					m_currentCompletion++;
					if (m_currentCompletion == m_completionList.size())
						m_currentCompletion = 0;
				}
				m_entry->SetText(m_precompletionStatement + m_completionList[m_currentCompletion]);
			}
			return true;
	}

	return false;
}

void LuaConsole::OnChange(const std::string &text) {
	m_completionList.clear();
}

void LuaConsole::OnEnter(const std::string &text) {
	if (!text.empty())
		ExecOrContinue(text);
	m_completionList.clear();
	Pi::ui->SelectWidget(m_entry);
	m_scroller->SetScrollPosition(1.0f);
}

void LuaConsole::UpdateCompletion(const std::string & statement) {
	// First, split the statement into chunks.
	m_completionList.clear();
	std::stack<std::string> chunks;
	bool method = false;
	bool expect_symbolname = false;
	std::string::const_iterator current_end = statement.end();
	std::string::const_iterator current_begin = statement.begin(); // To keep record when breaking off the loop.
	for (std::string::const_reverse_iterator r_str_it = statement.rbegin();
			r_str_it != statement.rend(); ++r_str_it) {
		if(Text::is_alphanumunderscore(*r_str_it)) {
			expect_symbolname = false;
			continue;
		} else if (expect_symbolname) // Wrong syntax.
			return;
		if(*r_str_it != '.' && (!chunks.empty() || *r_str_it != ':')) { // We are out of the expression.
			current_begin = r_str_it.base(); // Flag the symbol marking the beginning of the expression.
			break;
		}

		expect_symbolname = true; // We hit a separator, there should be a symbol name before it.
		chunks.push(std::string(r_str_it.base(), current_end));
		if (*r_str_it == ':') // If it is a colon, we know chunks is empty so it is incomplete.
			method = true;		// it must mean that we want to call a method.
		current_end = (r_str_it+1).base(); // +1 in order to point on the CURRENT character.
	}
	if (expect_symbolname) // Again, a symbol was expected when we broke out of the loop.
		return;

	if (current_begin != current_end)
		chunks.push(std::string(current_begin, current_end));

	if (chunks.empty()) {
		return;
	}

	lua_State * l = Lua::manager->GetLuaState();
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
	LuaObjectBase::GetNames(m_completionList, chunks.top(), method);
	if(!m_completionList.empty()) {
		std::sort(m_completionList.begin(), m_completionList.end());
		m_completionList.erase(std::unique(m_completionList.begin(), m_completionList.end()), m_completionList.end());
		// Add blank completion at the end of the list and point to it.
		m_currentCompletion = m_completionList.size();
		m_completionList.push_back("");

		m_precompletionStatement = statement;
	}
	lua_pop(l, lua_gettop(l)-stackheight); // Clean the whole stack.
}

void LuaConsole::AddOutput(const std::string &line) {
	m_output->AppendText(line + "\n");
}

void LuaConsole::ExecOrContinue(const std::string &stmt) {
	int result;
	lua_State *L = Lua::manager->GetLuaState();

    // If the statement is an expression, print its final value.
	result = luaL_loadbuffer(L, ("return " + stmt).c_str(), stmt.size()+7, CONSOLE_CHUNK_NAME);
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
				m_entry->SetText(stmt + "\n");
				lua_pop(L, 1);
				return;
			}
		}
	}

	if (result == LUA_ERRSYNTAX) {
		size_t msglen;
		const char *msg = lua_tolstring(L, -1, &msglen);
		AddOutput(std::string(msg, msglen));
		lua_pop(L, 1);
		return;
	}

	if (result == LUA_ERRMEM) {
		// this will probably fail too, since we've apparently
		// just had a memory allocation failure...
		AddOutput("memory allocation failure");
		return;
	}

	// set the global table
	lua_getfield(L, LUA_REGISTRYINDEX, "ConsoleGlobal");
	lua_setupvalue(L, -2, 1);

	std::istringstream stmt_stream(stmt);
	std::string string_buffer;

	std::getline(stmt_stream, string_buffer);
	AddOutput("> " + string_buffer);

	while(!stmt_stream.eof()) {
		std::getline(stmt_stream, string_buffer);
		AddOutput("  " + string_buffer);
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
				lua_pushvalue(L, top+i);

				result = lua_pcall(L, 1, 1, 0);
				size_t len = 0;
				const char *s = 0;
				if (result == 0)
					s = lua_tolstring(L, -1, &len);
				ss << s ? std::string(s, len) : "<internal error when converting result to string>";

				// pop the result
				lua_pop(L, 1);

				AddOutput(ss.str());
			}
		}
	}

	// pop all return values
	lua_settop(L, top);

	// update the history list

	if (! result) {
		// command succeeded... add it to the history unless it's just
		// an exact repeat of the immediate last command
		if (m_statementHistory.empty() || (stmt != m_statementHistory.back()))
			m_statementHistory.push_back(stmt);

		// clear the entry box
		m_entry->SetText("");
	}

	// always forget the history position and clear the stashed command
	m_historyPosition = -1;
	m_stashedStatement.clear();
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
static int l_console_addline(lua_State *L) {
	if (Pi::luaConsole) {
		size_t len;
		const char *s = luaL_checklstring(L, 1, &len);
		Pi::luaConsole->AddOutput(std::string(s, len));
	}
	return 0;
}

static int l_console_print(lua_State *L) {
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
		if (!str) { return luaL_error(L, "'tostring' must return a string to 'print'"); }
		if (i > 1) { line += '\t'; }
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
