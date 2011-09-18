#include "libs.h"
#include "Gui.h"
#include "TextureFont.h"

namespace Gui {

TextEntry::TextEntry()
{
	m_eventMask = EVENT_MOUSEDOWN;
	m_cursPos = 0;
	m_scroll = 0;
	m_font = 0;
}

TextEntry::TextEntry(TextureFont *font) {
	m_eventMask = EVENT_MOUSEDOWN;
	m_cursPos = 0;
	m_scroll = 0;
	m_font = font;
}

TextEntry::~TextEntry()
{
	m_clickout.disconnect();
}

void TextEntry::OnKeyPress(const SDL_keysym *sym)
{
	bool changed = false;
	Uint16 unicode = sym->unicode;
	if (sym->sym == SDLK_LEFT) SetCursorPos(m_cursPos-1);
	if (sym->sym == SDLK_RIGHT) SetCursorPos(m_cursPos+1);
	if (sym->sym == SDLK_BACKSPACE) {
		if (m_cursPos > 0) {
			m_text = m_text.substr(0, m_cursPos-1) + m_text.substr(m_cursPos);
			SetCursorPos(m_cursPos-1);
			changed = true;
		}
	}
	if (sym->sym == SDLK_DELETE) {
		if (m_cursPos < signed(m_text.size())) {
			m_text = m_text.substr(0, m_cursPos) + m_text.substr(m_cursPos+1);
			changed = true;
		}
	}
	if (isalnum(unicode) || (unicode == ' ') || (isgraph(unicode))) {
		char buf[2] = { char(unicode), 0 };
		m_text.insert(m_cursPos, std::string(buf));
		SetCursorPos(m_cursPos+1);
		changed = true;
	}

	onKeyPress.emit(sym);
	if (changed) onValueChanged.emit();
}

void TextEntry::GetSizeRequested(float size[2])
{
	// XXX this 1.5 should be PARAGRAPH_SPACING (currently #define'd in TextureFont.h)
	size[1] = 1.5*Gui::Screen::GetFontHeight(m_font) + 2.0;
}

bool TextEntry::OnMouseDown(MouseButtonEvent *e)
{
	m_clickout = RawEvents::onMouseDown.connect(sigc::mem_fun(this, &TextEntry::OnRawMouseDown));
	GrabFocus();
	m_justFocused = true;

	int i = Gui::Screen::PickCharacterInString(m_text, e->x - m_scroll, e->y, m_font);
	SetCursorPos(i);

	return false;
}

void TextEntry::OnRawMouseDown(MouseButtonEvent *e)
{
	if (!m_justFocused)
		Unfocus();
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
	glColor3f(1,0,0);
	Gui::Screen::MeasureString(m_text.substr(0, m_cursPos), curs_x, curs_y, m_font);
	if (curs_x - m_scroll > size[0]*0.75f) {
		m_scroll += size[0]*0.25f;
	} else if (curs_x - m_scroll < size[0]*0.25f) {
		m_scroll -= size[0]*0.25f;
		if (m_scroll < 0) m_scroll = 0;
	}

	glColor3f(0,0,0);
	glBegin(GL_TRIANGLE_FAN);
		glVertex2f(0,size[1]);
		glVertex2f(size[0],size[1]);
		glVertex2f(size[0],0);
		glVertex2f(0,0);
	glEnd();
	if (IsFocused()) glColor3f(1,1,1);
	else glColor3f(.75f, .75f, .75f);
	glBegin(GL_LINE_LOOP);
		glVertex2f(0,0);
		glVertex2f(size[0],0);
		glVertex2f(size[0],size[1]);
		glVertex2f(0,size[1]);
	glEnd();


	SetClipping(size[0], size[1]);
	Gui::Screen::RenderString(m_text, 1.0f - m_scroll, 1.0f, m_font);

	/* Cursor */
	glColor3f(0.5,0.5,0.5);
	glBegin(GL_LINES);
		glVertex2f(curs_x, 0);
		glVertex2f(curs_x, size[1]);
	glEnd();
	
	EndClipping();
}

} /* namespace Gui */
