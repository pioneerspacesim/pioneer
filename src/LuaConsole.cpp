#include "LuaConsole.h"
#include "LuaManager.h"
#include "Pi.h"
#include "gui/Gui.h"
#include "gui/GuiScreen.h"
#include "gui/GuiTextEntry.h"
#include "gui/GuiLabel.h"
#include "text/TextureFont.h"
#include "KeyBindings.h"
#include <sstream>

LuaConsole::LuaConsole(int displayedOutputLines):
	m_maxOutputLines(displayedOutputLines) {

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
}

LuaConsole::~LuaConsole() {}

bool LuaConsole::IsActive() const {
	return IsVisible() && m_entryField->IsFocused();
}

bool LuaConsole::OnFilterKeys(const SDL_keysym *sym) {
	return !KeyBindings::toggleLuaConsole.binding.Matches(sym);
}

void LuaConsole::OnKeyPressed(const SDL_keysym *sym) {
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

	if (((sym->unicode == '\n') || (sym->unicode == '\r')) && ((sym->mod & KMOD_CTRL) == 0)) {
		ExecOrContinue();
	}
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
	lua_State *L = Pi::luaManager->GetLuaState();

	result = luaL_loadbuffer(L, stmt.c_str(), stmt.size(), "console");

	// check for an incomplete statement
	// (follows logic from the official Lua interpreter lua.c:incomplete())
	if (result == LUA_ERRSYNTAX) {
		const char eofstring[] = LUA_QL("<eof>");
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

void LuaConsole::Register()
{
	lua_State *l = Pi::luaManager->GetLuaState();

	LUA_DEBUG_START(l);

	static const luaL_reg methods[] = {
		{ "AddLine", l_console_addline },
		{ 0, 0 }
	};

	luaL_register(l, "Console", methods);
	lua_pop(l, 1);

	LUA_DEBUG_END(l, 0);
}
