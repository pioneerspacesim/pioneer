#ifndef _GUITEXTENTRY_H
#define _GUITEXTENTRY_H

#include "GuiWidget.h"
#include <string>

namespace Gui {
	class TextEntry: public Widget {
	public:
		TextEntry();
		virtual ~TextEntry();
		virtual void GetSizeRequested(float size[2]);
		virtual void Draw();
		virtual bool OnMouseDown(MouseButtonEvent *e);
		void SetText(const std::string &text) {
			m_text = text;
			SetCursorPos(m_text.size());
		}
		std::string GetText() const { return m_text; }
		void SetCursorPos(int pos) { m_cursPos = CLAMP(pos, 0, (signed)m_text.size()); }
	private:
		void OnRawKeyDown(SDL_KeyboardEvent *);

		std::string m_text;
		int m_cursPos;
		int m_scroll;
		sigc::connection m_rawKbDownCon;
	};
}

#endif /* _GUITEXTENTRY_H */
