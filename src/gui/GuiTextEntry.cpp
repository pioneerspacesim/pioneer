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

bool TextEntry::OnKeyPress(const SDL_keysym *sym)
{
	bool accepted = onFilterKeys.empty() ? true : onFilterKeys.emit(sym);
	if (! accepted)
		return false;
	accepted = false;

	bool changed = false;
	Uint16 unicode = sym->unicode;

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

	if ((unicode == '\n') || (unicode == '\r')) {
		switch (m_newlineMode) {
		case IgnoreNewline:
			unicode = '\0';
			break;
		case AcceptNewline:
			unicode = '\n';
			break;
		case AcceptCtrlNewline:
			unicode = (sym->mod & KMOD_CTRL) ? '\n' : '\0';
			break;
		}
	}

	if (isgraph(unicode) || (unicode == ' ') || (unicode == '\n')) {
		if (unicode == '\n')
			++m_newlineCount;
		char buf[4];
		int len = Text::utf8_encode_char(unicode, buf);
		m_text.insert(m_cursPos, buf, len);
		SetCursorPos(m_cursPos+len);
		changed = true;
		accepted = true;
	}

	if (oldNewlineCount != m_newlineCount)
		ResizeRequest();

	onKeyPress.emit(sym);
	if (changed) onValueChanged.emit();

	return accepted;
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

	glColor3f(0,0,0);
	glBegin(GL_TRIANGLE_FAN);
		glVertex2f(0,size[1]);
		glVertex2f(size[0],size[1]);
		glVertex2f(size[0],0);
		glVertex2f(0,0);
	glEnd();

	Color c = IsFocused() ? Color::WHITE : Color(0.75f,0.75f,0.75f,1.0f);

	glColor4fv(c);
	glBegin(GL_LINE_LOOP);
		glVertex2f(0,0);
		glVertex2f(size[0],0);
		glVertex2f(size[0],size[1]);
		glVertex2f(0,size[1]);
	glEnd();


	SetScissor(true);

	Gui::Screen::RenderString(m_text, 1.0f - m_scroll, 0.0f, c, m_font.Get());

	/* Cursor */
	glColor3f(0.5f,0.5f,0.5f);
	glBegin(GL_LINES);
		glVertex2f(curs_x + 1.0f - m_scroll, curs_y + Gui::Screen::GetFontDescender(m_font.Get()) - Gui::Screen::GetFontHeight(m_font.Get()));
		glVertex2f(curs_x + 1.0f - m_scroll, curs_y + Gui::Screen::GetFontDescender(m_font.Get()));
	glEnd();

	SetScissor(false);
}

} /* namespace Gui */
