#ifndef _GUILABEL_H
#define _GUILABEL_H

#include "GuiWidget.h"
#include "GuiTextLayout.h"
#include <string>

class TextureFont;

namespace Gui {
	class Label: public Widget {
	public:
		Label(const char *text, TextLayout::ColourMarkupMode colourMarkupMode = TextLayout::ColourMarkupUse);
		Label(const std::string &text, TextLayout::ColourMarkupMode colourMarkupMode = TextLayout::ColourMarkupUse);
		virtual void Draw();
		virtual ~Label();
		virtual void GetSizeRequested(float size[2]);
		void SetText(const char *text);
		void SetText(const std::string &text);
		Label *Shadow(bool isOn) { m_shadow = isOn; return this; }
		Label *Color(const float rgb[3]);
		Label *Color(float r, float g, float b);
		Label *Color(const ::Color &);
	private:
		void Init(const std::string &text, TextLayout::ColourMarkupMode colourMarkupMode);
		void UpdateLayout();
		void RecalcSize();
		std::string m_text;
		::Color m_color;
		bool m_shadow;
		GLuint m_dlist;
		RefCountedPtr<TextureFont> m_font;
		TextLayout *m_layout;
		TextLayout::ColourMarkupMode m_colourMarkupMode;
	};
}

#endif /* _GUILABEL_H */
