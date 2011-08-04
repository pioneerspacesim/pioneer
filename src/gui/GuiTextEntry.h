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
		void SetCursorPos(int pos) { m_cursPos = Clamp(pos, 0, signed(m_text.size())); }
		virtual void OnKeyPress(const SDL_keysym *);
		virtual void Show() { GrabFocus(); Widget::Show(); }

		sigc::signal<void, const SDL_keysym*> onKeyPress;
		sigc::signal<void> onValueChanged;
	private:

		void OnRawMouseDown(MouseButtonEvent *e);
		void Unfocus();

		std::string m_text;
		int m_cursPos;
		int m_scroll;

		bool m_justFocused;
		sigc::connection m_clickout;
	};
}

#endif /* _GUITEXTENTRY_H */
