#include "LuaConsole.h"
#include "LuaManager.h"
#include "Pi.h"
#include "gui/Gui.h"
#include "gui/GuiScreen.h"
#include "gui/GuiTextEntry.h"
#include "gui/GuiLabel.h"
#include "TextureFont.h"
#include "FontManager.h"
#include "KeyBindings.h"
#include <sstream>

LuaConsole::LuaConsole(int displayedOutputLines):
	m_maxOutputLines(displayedOutputLines) {

	this->SetTransparency(false);
	this->SetBgColor(0.6f, 0.1f, 0.0f, 0.6f);

	m_font = Gui::Screen::GetFontManager()->GetTextureFont("ConsoleFont");
	m_entryField = new Gui::TextEntry(m_font);
	m_entryField->SetNewlineMode(Gui::TextEntry::AcceptCtrlNewline);
	m_outputLines.reserve(displayedOutputLines);
	m_nextOutputLine = 0;

	// XXX HACK: bypassing TextEntry::Show, because it grabs focus
	m_entryField->Gui::Widget::Show();
	m_entryField->onFilterKeys.connect(sigc::mem_fun(this, &LuaConsole::onFilterKeys));
	m_entryField->onKeyPress.connect(sigc::mem_fun(this, &LuaConsole::onKeyPressed));

	this->PackEnd(m_entryField);
}

LuaConsole::~LuaConsole() {
	delete m_entryField;
	for (std::vector<Gui::Label*>::const_iterator
		it = m_outputLines.begin(); it != m_outputLines.end(); ++it) {
		delete (*it);
	}
	m_outputLines.clear();
}

bool LuaConsole::onFilterKeys(const SDL_keysym *sym) {
	return !KeyBindings::toggleLuaConsole.binding.Matches(sym);
}

void LuaConsole::onKeyPressed(const SDL_keysym *sym) {
	// XXX totally horrible doing this on every key press
	ResizeRequest();

	if (((sym->unicode == '\n') || (sym->unicode == '\r')) && ((sym->mod & KMOD_CTRL) == 0)) {
		execOrContinue();
	}
}

void LuaConsole::addOutput(const std::string &line) {
	Gui::Label *label = 0;
	if (int(m_outputLines.size()) > m_nextOutputLine) {
		label = m_outputLines[m_nextOutputLine];
		this->Remove(label);
	} else {
		label = new Gui::Label(m_font);
		m_outputLines.push_back(label);
	}
	m_nextOutputLine = (m_nextOutputLine + 1) % m_maxOutputLines;

	label->SetText(line);
	float size[2];
	label->GetSizeRequested(size);
	label->SetSize(float(Gui::Screen::GetWidth()), size[1]);
	label->Show();

	this->Remove(m_entryField);
	this->PackEnd(label);
	this->PackEnd(m_entryField);
}

void LuaConsole::execOrContinue() {
	const std::string stmt = m_entryField->GetText();
	int result;
	lua_State *L = Pi::luaManager.GetLuaState();

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
				lua_pop(L, 1);
				return;
			}
		}
	}

	if (result == LUA_ERRSYNTAX) {
		size_t msglen;
		const char *msg = lua_tolstring(L, -1, &msglen);
		addOutput(std::string(msg, msglen));
		lua_pop(L, 1);
		return;
	}

	if (result == LUA_ERRMEM) {
		// this will probably fail too, since we've apparently
		// just had a memory allocation failure...
		addOutput("memory allocation failure");
		return;
	}

	// perform a protected call
	int top = lua_gettop(L) - 1; // -1 for the chunk itself
	result = lua_pcall(L, 0, LUA_MULTRET, 0);

	if (result == LUA_ERRRUN) {
		size_t len;
		const char *s = lua_tolstring(L, -1, &len);
		addOutput(std::string(s, len));
	} else if (result == LUA_ERRERR) {
		size_t len;
		const char *s = lua_tolstring(L, -1, &len);
		addOutput("error in error handler: " + std::string(s, len));
	} else if (result == LUA_ERRMEM) {
		addOutput("memory allocation failure");
	} else {
		int nresults = lua_gettop(L) - top;
		if (nresults) {
			lua_getglobal(L, "tostring");
			std::ostringstream ss;
			for (int i = 0; i < nresults; ++i) {
				ss.str(std::string());
				if (nresults > 1)
					ss << "[" << i << "] ";

				lua_pushvalue(L, -1); // duplicate the tostring function
				lua_pushvalue(L, top+i);
				if (lua_pcall(L, 1, 1, 0)) {
					ss << "<internal error when converting result to string>";
				} else {
					size_t len;
					const char *s = lua_tolstring(L, -1, &len);
					ss << std::string(s, len);
				}

				addOutput(ss.str());
			}
		}
	}

	// pop all return values
	lua_settop(L, top);

	if (! result) {
		m_entryField->SetText("");
		ResizeRequest();
	}
}

static int l_console_print(lua_State *L) {
	if (Pi::luaConsole) {
		size_t len;
		const char *s = luaL_checklstring(L, 1, &len);
		Pi::luaConsole->addOutput(std::string(s, len));
	}
	return 0;
}

void LuaConsole::Register()
{
	lua_State *l = Pi::luaManager.GetLuaState();

	LUA_DEBUG_START(l);

	static const luaL_reg methods[] = {
		{ "AddLine", l_console_print },
		{ 0, 0 }
	};

	luaL_register(l, "Console", methods);
	lua_pop(l, 1);

	LUA_DEBUG_END(l, 0);
}
