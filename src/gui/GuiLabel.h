#ifndef _GUILABEL_H
#define _GUILABEL_H

#include "GuiWidget.h"
#include <string>

class TextLayout;

namespace Gui {
	class Label: public Widget {
	public:
		Label(const char *text);
		Label(const std::string &text);
		virtual void Draw();
		virtual ~Label();
		virtual void GetSizeRequested(float size[2]);
		void SetText(const char *text);
		void SetText(const std::string text);
		Label *Shadow(bool isOn) { m_shadow = isOn; return this; }
		Label *Color(const float rgb[3]);
		Label *Color(float r, float g, float b);
		Label *Color(const ::Color &);
	private:
		void UpdateLayout();
		void RecalcSize();
		std::string m_text;
		::Color m_color;
		bool m_shadow;
		GLuint m_dlist;
		TextureFont *m_font;
		TextLayout *m_layout;
	};
}

#endif /* _GUILABEL_H */
