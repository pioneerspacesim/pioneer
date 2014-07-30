// Copyright © 2008-2014 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#include "libs.h"
#include "Gui.h"
#include "text/TextureFont.h"
#include "text/TextSupport.h"

namespace Gui {

TextEntry::TextEntry()
{
	m_eventMask = EVENT_MOUSEDOWN;
	m_cursPos = 0;
	m_scroll = 0;
	m_font = Gui::Screen::GetFont();
	m_newlineMode = IgnoreNewline;
	m_newlineCount = 0;
}

TextEntry::~TextEntry()
{
	m_clickout.disconnect();
}

void TextEntry::SetText(const std::string &text)
{
	m_text = text;
	SetCursorPos(m_text.size());

	int count = 0;
	for (int i = 0; i < int(text.size()); ++i) {
		if (text[i] == '\n')
			++count;
	}
	m_newlineCount = count;
	ResizeRequest();
}

bool TextEntry::OnKeyDown(const SDL_Keysym *sym)
{
	bool accepted = false;
	bool changed = false;

	int oldNewlineCount = m_newlineCount;

	// XXX moving the cursor is not UTF-8 safe
	if (sym->sym == SDLK_LEFT || sym->sym == SDLK_RIGHT) {
		bool forward = (sym->sym == SDLK_RIGHT);
		int direction = (forward) ? 1 : -1;
		if (!(sym->mod & KMOD_CTRL)) {
			SetCursorPos(m_cursPos + direction);
		} else {
			int inspect_offset = (forward) ? 0 : -1; // When going back, we need the character before the cursor.
			int ending = (forward) ? m_text.size() : 0;
			int current = m_cursPos+inspect_offset;
			bool found_word = false;

			while(current != ending) {
				bool alphanum;

				alphanum = Text::is_alphanumunderscore(m_text[current]);

				if (found_word && !alphanum) { // Word boundary.
					current -= inspect_offset; // Make up for the initial offset.
					break;
				}
				current += direction;
				found_word = found_word || alphanum; // You need to be in a word before finding its boudaries.
			}
			SetCursorPos(current);
		}
		accepted = true;
	}

	// XXX deleting characters is not UTF-8 safe
	if (sym->sym == SDLK_BACKSPACE) {
		if (m_cursPos > 0) {
			if (m_text[m_cursPos-1] == '\n')
				--m_newlineCount;
			m_text = m_text.substr(0, m_cursPos-1) + m_text.substr(m_cursPos);
			SetCursorPos(m_cursPos-1);
			changed = true;
		}
		accepted = true;
	}
	if (sym->sym == SDLK_DELETE) {
		if (m_cursPos < signed(m_text.size())) {
			if (m_text[m_cursPos] == '\n')
				--m_newlineCount;
			m_text = m_text.substr(0, m_cursPos) + m_text.substr(m_cursPos+1);
			changed = true;
		}
		accepted = true;
	}

	if (sym->sym == SDLK_HOME) {
		size_t pos = m_text.rfind('\n', std::max(m_cursPos-1, 0));
		if (pos == std::string::npos)
			pos = 0;
		else
			++pos;
		m_cursPos = int(pos);
		accepted = true;
	}
	if (sym->sym == SDLK_END) {
		size_t pos = m_text.find('\n', m_cursPos);
		if (pos == std::string::npos)
			pos = m_text.size();
		m_cursPos = int(pos);
		accepted = true;
	}
	if (sym->sym == SDLK_RETURN) {
		switch (m_newlineMode) {
			case IgnoreNewline:
				accepted = false;
				break;
			case AcceptNewline:
				accepted = true;
				break;
			case AcceptCtrlNewline:
				accepted = sym->mod & KMOD_CTRL;
				break;
		}
		if (accepted) {
			++m_newlineCount;
			OnTextInput('\n');
		}
	}

	if (oldNewlineCount != m_newlineCount)
		ResizeRequest();

	onKeyPress.emit(sym);
	if (changed) onValueChanged.emit();

	return accepted;
}

void TextEntry::OnTextInput(Uint32 unicode)
{
	bool changed = false;

	if (isgraph(unicode) || (unicode == ' ') || (unicode == '\n')) {
		char buf[4];
		int len = Text::utf8_encode_char(unicode, buf);
		m_text.insert(m_cursPos, buf, len);
		SetCursorPos(m_cursPos+len);
		changed = true;
	}

	if (changed) onValueChanged.emit();
}

void TextEntry::GetSizeRequested(float size[2])
{
	size[1] = Gui::Screen::GetFontHeight(m_font.Get()) * (m_newlineCount+1) + Gui::Screen::GetFontDescender(m_font.Get());
}

bool TextEntry::OnMouseDown(MouseButtonEvent *e)
{
	if (e->button == SDL_BUTTON_LEFT) {
		m_clickout = RawEvents::onMouseDown.connect(sigc::mem_fun(this, &TextEntry::OnRawMouseDown));
		GrabFocus();
		m_justFocused = true;

		int i = Gui::Screen::PickCharacterInString(m_text, e->x - m_scroll, e->y, m_font.Get());
		SetCursorPos(i);

		return false;
	} else
		return true;
}

void TextEntry::OnRawMouseDown(MouseButtonEvent *e)
{
	if (!m_justFocused)
		Unfocus();
}

void TextEntry::GrabFocus()
{
	Screen::SetFocused(this, true);
	// XXX should this be here? or somewhere else?
	// In some places (at least the Lua console and the sector view search box),
	// a keyboard shortcut is used to switch to the text entry widget.
	// Pressing '`' opens the console, pressing '/' in the sector view selects the search box.
	// Those are normal text keys, so SDL generates an SDL_TEXTINPUT event for them.
	// But we don't want to capture that text, because it's not really text input
	// (it happens "before" the text entry widget gets focus)
	// So we flush those events from the queue here.
	SDL_FlushEvents(SDL_TEXTEDITING, SDL_TEXTINPUT);
}

void TextEntry::Unfocus()
{
	if (!Screen::IsFocused(this))
		return;
	Screen::ClearFocus();
	m_clickout.disconnect();
	SetCursorPos(0);
}

void TextEntry::Draw()
{
	PROFILE_SCOPED()
	m_justFocused = false;

	float size[2];
	GetSize(size);

	// find cursor position
	float curs_x, curs_y;
	Gui::Screen::MeasureCharacterPos(m_text, m_cursPos, curs_x, curs_y, m_font.Get());

	if (curs_x - m_scroll > size[0]*0.75f) {
		m_scroll += int(size[0]*0.25f);
	} else if (curs_x - m_scroll < size[0]*0.25f) {
		m_scroll -= int(size[0]*0.25f);
		if (m_scroll < 0) m_scroll = 0;
	}

	//background
	Theme::DrawRect(vector2f(0.f), vector2f(size[0], size[1]), Color(0,0,0,192), Screen::alphaBlendState);

	//outline
	const Color c = IsFocused() ? Color::WHITE : Color(192, 192, 192, 255);
	const vector3f boxVts[] = {
		vector3f(0.f, 0.f, 0.f),
		vector3f(size[0],0.f, 0.f),
		vector3f(size[0],size[1], 0.f),
		vector3f(0,size[1], 0.f)
	};
	Screen::GetRenderer()->DrawLines(4, &boxVts[0], c, Screen::alphaBlendState, Graphics::LINE_LOOP);

	//text
	SetScissor(true);
	Gui::Screen::RenderString(m_text, 1.0f - m_scroll, 0.0f, c, m_font.Get());
	SetScissor(false);

	//cursor
	const vector3f cursorVts[] = {
		vector3f(curs_x + 1.0f - m_scroll, curs_y + Gui::Screen::GetFontDescender(m_font.Get()) - Gui::Screen::GetFontHeight(m_font.Get()), 0.f),
		vector3f(curs_x + 1.0f - m_scroll, curs_y + Gui::Screen::GetFontDescender(m_font.Get()), 0.f),
	};
	Screen::GetRenderer()->DrawLines(2, &cursorVts[0], Color(128), Screen::alphaBlendState);
}

} /* namespace Gui */
