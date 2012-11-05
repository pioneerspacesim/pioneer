// Copyright © 2008-2012 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#include "TextEntry.h"
#include "Context.h"
#include "text/TextureFont.h"
#include "text/TextSupport.h"

namespace UI {

TextEntry::TextEntry(Context *context, const std::string &text) : Container(context),
	m_cursor(0)
{
	m_label = GetContext()->Label(text);
	AddWidget(m_label);
}

Point TextEntry::PreferredSize()
{
	const Point labelPreferredSize(m_label->PreferredSize());
	const Point borderSize(GetContext()->GetSkin().BackgroundNormal().borderWidth*2);
	return SizeAdd(labelPreferredSize, borderSize);
}

void TextEntry::Layout()
{
	const Uint32 borderWidth = GetContext()->GetSkin().BackgroundNormal().borderWidth;

	const Point &size = GetSize();

	const Point innerPos(borderWidth, borderWidth);
	const Point innerSize(size.x - borderWidth*2, size.y - borderWidth*2);

	SetWidgetDimensions(m_label, innerPos, innerSize);

	// XXX see ::Draw. after Container::Draw we're still translated to the
	// label origin so we calculate the cursor from there
	const float cursorTop    = 0.0f;
	const float cursorBottom = m_label->GetSize().y;

	m_cursorVertices[0] = vector3f(0.0f, cursorTop,    0.0f);
	m_cursorVertices[1] = vector3f(0.0f, cursorBottom, 0.0f);

	m_label->Layout();
}

void TextEntry::Update()
{
	float cursorLeft, cursorBaseline;
	GetContext()->GetFont(GetFont())->MeasureCharacterPos(GetText().c_str(), m_cursor, cursorLeft, cursorBaseline);
	m_cursorVertices[0].x = m_cursorVertices[1].x = cursorLeft + 0.5f;

	// offset such that the cursor is always visible
	const Point offset(m_label->GetDrawOffset());
	const Point size(m_label->GetSize());

	const int windowLeft = -offset.x;
	const int windowRight = windowLeft + size.x;

	const int cursorPos = roundf(cursorLeft);

	if (cursorPos > windowRight)
		m_label->SetDrawOffset(Point(-cursorPos+size.x-1, 0.0f));
	else if (cursorPos < windowLeft)
		m_label->SetDrawOffset(Point(-cursorPos, 0.0f));
}

void TextEntry::Draw()
{
	if (IsSelected())
		GetContext()->GetSkin().DrawBackgroundActive(Point(), GetSize());
	else
		GetContext()->GetSkin().DrawBackgroundNormal(Point(), GetSize());

	Container::Draw();

	if (IsSelected())
		GetContext()->GetRenderer()->DrawLines(2, m_cursorVertices, Color::WHITE);
}

TextEntry *TextEntry::SetText(const std::string &text)
{
	m_label->SetText(text);
	GetContext()->RequestLayout();
	return this;
}

void TextEntry::HandleKeyPress(const KeyboardEvent &event)
{
	std::string text(m_label->GetText());

	switch (event.keysym.sym) {
		case SDLK_LEFT:
			if (m_cursor > 0) {
				const char *cstr = text.c_str();
				m_cursor -= Text::utf8_prev_char_offset(cstr + m_cursor, cstr);
			}
			break;

		case SDLK_RIGHT:
			if (m_cursor < text.size()) {
				m_cursor += Text::utf8_next_char_offset(text.c_str() + m_cursor);
			}
			break;

		case SDLK_HOME:
			m_cursor = 0;
			break;

		case SDLK_END:
			m_cursor = text.size();
			break;

		case SDLK_BACKSPACE:
			if (text.size() > 0 && m_cursor > 0) {
				const char *cstr = text.c_str();
				const int len = Text::utf8_prev_char_offset(cstr + m_cursor, cstr);
				m_cursor -= len;
				text.erase(m_cursor, len);

				m_label->SetText(text);
				onChange.emit(text);
			}
			break;

		case SDLK_DELETE:
			if (text.size() > m_cursor) {
				const int len = Text::utf8_next_char_offset(text.c_str() + m_cursor);
				text.erase(m_cursor, len);

				m_label->SetText(text);
				onChange.emit(text);
			}
			break;

		case SDLK_RETURN:
			GetContext()->DeselectWidget(this);
			onEnter.emit(text);
			break;

		default:

			if (event.keysym.mod & KMOD_CTRL) {
				switch (event.keysym.sym) {
					case SDLK_u:
						m_cursor = 0;
						m_label->SetText("");
						onChange.emit("");
						break;

					case SDLK_w: {
						size_t pos = text.find_last_not_of(' ', m_cursor);
						if (pos != std::string::npos) pos = text.find_last_of(' ', pos);
						m_cursor = pos != std::string::npos ? pos+1 : 0;
						text.erase(m_cursor);
						m_label->SetText(text);
						onChange.emit(text);
						break;
					}

					default:
						break;
				}
			}

			// ignore non-shift meta
			else if (event.keysym.mod & ~KMOD_SHIFT)
				return;

			// naively accept anything outside C0 and C1. probably safe enough for
			// now, but needs revisiting if we one day support rtl, cjk, etc
			if ((event.keysym.unicode > 0x1f && event.keysym.unicode < 0x7f) || event.keysym.unicode > 0x9f) {
				char buf[4] = {};
				const int len = Text::utf8_encode_char(event.keysym.unicode, buf);
				text.insert(m_cursor, buf, len);
				m_cursor += len;

				m_label->SetText(text);
				onChange.emit(text);
			}

			break;
	}
}

}
