#include "LuaConsole.h"
#include "Pi.h"
#include "gui/Gui.h"
#include "gui/GuiScreen.h"
#include "gui/GuiTextEntry.h"
#include "gui/GuiLabel.h"
#include "TextureFont.h"
#include "FontManager.h"
#include "KeyBindings.h"

LuaConsole::LuaConsole(int displayedOutputLines):
	m_maxOutputLines(displayedOutputLines) {

	float size[2];

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
		addOutput(m_entryField->GetText());
		m_entryField->SetText("");
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
