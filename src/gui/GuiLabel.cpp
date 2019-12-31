// Copyright © 2008-2020 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#include "Gui.h"

#include "graphics/Renderer.h"

namespace Gui {

	Label::Label(const char *text, TextLayout::ColourMarkupMode colourMarkupMode)
	{
		Init(std::string(text), colourMarkupMode);
	}

	Label::Label(const std::string &text, TextLayout::ColourMarkupMode colourMarkupMode)
	{
		Init(text, colourMarkupMode);
	}

	Label::~Label()
	{
		m_layout.reset();
	}

	void Label::Init(const std::string &text, TextLayout::ColourMarkupMode colourMarkupMode)
	{
		m_needsUpdate = true;
		m_colourMarkupMode = colourMarkupMode;
		m_shadow = false;
		m_layout = 0;
		m_dlist = 0;
		m_font = Gui::Screen::GetFont();
		m_color = ::Color::WHITE;
		UpdateLayout();
		SetText(text);
	}

	void Label::UpdateLayout()
	{
		if (!m_layout.get() || m_needsUpdate) {
			m_needsUpdate = false;
			m_layout.reset(new TextLayout(m_text.c_str(), m_font, m_colourMarkupMode));
		}
	}

	void Label::RecalcSize()
	{
		ResizeRequest();
	}

	Label *Label::Color(Uint8 r, Uint8 g, Uint8 b)
	{
		::Color c(r, g, b);
		if (m_color != c) {
			m_color = c;
		}
		return this;
	}

	Label *Label::Color(const ::Color &c)
	{
		if (m_color != c) {
			m_color = c;
		}
		return this;
	}

	void Label::SetText(const char *text)
	{
		SetText(std::string(text));
	}

	void Label::SetText(const std::string &text)
	{
		if (m_text != text || m_needsUpdate) {
			m_text = text;
			m_layout->SetText(m_text.c_str());
			RecalcSize();
		}
	}

	void Label::Draw()
	{
		PROFILE_SCOPED()
		if (!m_layout || m_needsUpdate)
			UpdateLayout();

		// the size might not have bene updated, poke it until it is
		float size[2];
		GetSize(size);
		if (is_equal_exact(size[0], 0.0f))
			RecalcSize();
		m_layout->Update(size[0], m_color);

		if (m_shadow) {
			Graphics::Renderer *r = Gui::Screen::GetRenderer();
			r->Translate(1, 1, 0);
			m_layout->Render(size[0], Color::BLACK);
			r->Translate(-1, -1, 0);
		}
		m_layout->Render(size[0], m_color);
	}

	void Label::GetSizeRequested(float size[2])
	{
		m_layout->MeasureSize(size[0], size);
	}

} // namespace Gui
