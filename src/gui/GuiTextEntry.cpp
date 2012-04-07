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
	if (sym->sym == SDLK_LEFT) {
		SetCursorPos(m_cursPos-1);
		accepted = true;
	}
	if (sym->sym == SDLK_RIGHT) {
		SetCursorPos(m_cursPos+1);
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
		int len = Text::conv_wc_to_mb(unicode, buf);
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
	// XXX this 1.5f should be PARAGRAPH_SPACING (currently #define'd in Text::TextureFont.h)
	size[1] = (m_newlineCount*1.5f+1.0f)*Gui::Screen::GetFontHeight(m_font.Get()) + 2.0f;
}

bool TextEntry::OnMouseDown(MouseButtonEvent *e)
{
	m_clickout = RawEvents::onMouseDown.connect(sigc::mem_fun(this, &TextEntry::OnRawMouseDown));
	GrabFocus();
	m_justFocused = true;

	int i = Gui::Screen::PickCharacterInString(m_text, e->x - m_scroll, e->y, m_font.Get());
	SetCursorPos(i);

	return false;
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

	Gui::Screen::RenderString(m_text, 1.0f - m_scroll, 1.0f, c, m_font.Get());

	/* Cursor */
	glColor3f(0.5f,0.5f,0.5f);
	glBegin(GL_LINES);
		glVertex2f(curs_x + 1.0f - m_scroll, curs_y - Gui::Screen::GetFontHeight(m_font.Get()) - 1.0f);
		glVertex2f(curs_x + 1.0f - m_scroll, curs_y + 1.0f);
	glEnd();
	
	SetScissor(false);
}

} /* namespace Gui */
