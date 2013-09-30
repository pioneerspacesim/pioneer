// Copyright © 2008-2013 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#include "LuaConsole.h"
#include "LuaManager.h"
#include "Pi.h"
#include "gui/Gui.h"
#include "gui/GuiScreen.h"
#include "gui/GuiTextEntry.h"
#include "gui/GuiLabel.h"
#include "text/TextureFont.h"
#include "text/TextSupport.h"
#include "KeyBindings.h"
#include <sstream>
#include <stack>
#include <algorithm>

#define TRUSTED_CONSOLE 1

#if TRUSTED_CONSOLE
static const char CONSOLE_CHUNK_NAME[] = "[T] console";
#else
static const char CONSOLE_CHUNK_NAME[] = "console";
#endif

LuaConsole::LuaConsole(int displayedOutputLines):
	m_maxOutputLines(displayedOutputLines),
	m_precompletionStatement(),
	m_completionList() {

	m_historyPosition = -1;

	SetTransparency(false);
	SetBgColor(0.6f, 0.1f, 0.0f, 0.6f);

	Gui::Screen::PushFont("ConsoleFont");
	m_entryField = new Gui::TextEntry();
	Gui::Screen::PopFont();
	m_entryField->SetNewlineMode(Gui::TextEntry::AcceptCtrlNewline);
	m_outputLines.reserve(displayedOutputLines);
	m_nextOutputLine = 0;

	// XXX HACK: bypassing TextEntry::Show, because it grabs focus
	m_entryField->Gui::Widget::Show();
	m_entryField->onFilterKeys.connect(sigc::mem_fun(this, &LuaConsole::OnFilterKeys));
	m_entryField->onKeyPress.connect(sigc::mem_fun(this, &LuaConsole::OnKeyPressed));

	PackEnd(m_entryField);

	// prepare the global table
	lua_State *l = Lua::manager->GetLuaState();

	LUA_DEBUG_START(l);

	lua_newtable(l);
	lua_newtable(l);
	lua_pushliteral(l, "__index");
	lua_getglobal(l, "_G");
	lua_rawset(l, -3);
	lua_setmetatable(l, -2);
	lua_setfield(l, LUA_REGISTRYINDEX, "ConsoleGlobal");

	LUA_DEBUG_END(l, 0);
}

LuaConsole::~LuaConsole() {}

bool LuaConsole::IsActive() const {
	return IsVisible() && m_entryField->IsFocused();
}

bool LuaConsole::OnFilterKeys(const SDL_Keysym *sym) {
	return !KeyBindings::toggleLuaConsole.binding.Matches(sym);
}

void LuaConsole::OnKeyPressed(const SDL_Keysym *sym) {
	// XXX totally horrible doing this on every key press
	ResizeRequest();

	if ((sym->sym == SDLK_UP) || (sym->sym == SDLK_DOWN)) {
		if (m_historyPosition == -1) {
			if (sym->sym == SDLK_UP) {
				m_historyPosition = (m_statementHistory.size() - 1);
				if (m_historyPosition != -1) {
					m_stashedStatement = m_entryField->GetText();
					m_entryField->SetText(m_statementHistory[m_historyPosition]);
					ResizeRequest();
				}
			}
		} else {
			if (sym->sym == SDLK_DOWN) {
				++m_historyPosition;
				if (m_historyPosition >= int(m_statementHistory.size())) {
					m_historyPosition = -1;
					m_entryField->SetText(m_stashedStatement);
					m_stashedStatement.clear();
					ResizeRequest();
				} else {
					m_entryField->SetText(m_statementHistory[m_historyPosition]);
					ResizeRequest();
				}
			} else {
				if (m_historyPosition > 0) {
					--m_historyPosition;
					m_entryField->SetText(m_statementHistory[m_historyPosition]);
					ResizeRequest();
				}
			}
		}
	}

	// CTRL+U clears the current command
	if ((sym->sym == SDLK_u) && (sym->mod & KMOD_CTRL)) {
		m_stashedStatement.clear();
		m_entryField->SetText("");
		m_historyPosition = -1;
		ResizeRequest();
	}

	if (sym->sym == SDLK_TAB) {
		if (m_completionList.empty()) {
			UpdateCompletion(m_entryField->GetText());
		}
		if (!m_completionList.empty()) { // We still need to test whether it failed or not.
			if (sym->mod & KMOD_SHIFT) {
				if (m_currentCompletion == 0)
					m_currentCompletion = m_completionList.size();
				m_currentCompletion--;
			} else {
				m_currentCompletion++;
				if (m_currentCompletion == m_completionList.size())
					m_currentCompletion = 0;
			}
			m_entryField->SetText(m_precompletionStatement + m_completionList[m_currentCompletion]);
			ResizeRequest();
		}
	} else if (!m_completionList.empty()) { // XXX SDL2 can't really range check keysyms anymore ...    && (sym->sym < SDLK_NUMLOCK || sym->sym > SDLK_COMPOSE)) {
		m_completionList.clear();
	}

	if (sym->sym == SDLK_RETURN && !(sym->mod & KMOD_CTRL)) {
		ExecOrContinue();
	}
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
	Gui::Label *label = 0;
	if (int(m_outputLines.size()) > m_nextOutputLine) {
		label = m_outputLines[m_nextOutputLine];
		Remove(label);
	} else {
		Gui::Screen::PushFont("ConsoleFont");
		label = new Gui::Label("", Gui::TextLayout::ColourMarkupNone);
		Gui::Screen::PopFont();
		m_outputLines.push_back(label);
	}
	m_nextOutputLine = (m_nextOutputLine + 1) % m_maxOutputLines;

	label->SetText(line);
	float size[2];
	label->GetSizeRequested(size);
	label->SetSize(float(Gui::Screen::GetWidth()), size[1]);
	label->Show();

	Remove(m_entryField);
	PackEnd(label);
	PackEnd(m_entryField);
}

void LuaConsole::ExecOrContinue() {
	const std::string stmt = m_entryField->GetText();
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
				m_entryField->SetText(stmt + "\n");
				m_entryField->ResizeRequest();
				ResizeRequest();
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
		m_entryField->SetText("");
		ResizeRequest();
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
	printf("%s\n", line.c_str());
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
