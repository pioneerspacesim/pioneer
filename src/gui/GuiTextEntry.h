#ifndef _GUITEXTENTRY_H
#define _GUITEXTENTRY_H

#include "GuiWidget.h"
#include <string>

namespace Text { class TextureFont; }

namespace Gui {
	class TextEntry: public Widget {
	public:
		enum NewlineMode {
			IgnoreNewline,
			AcceptNewline,
			AcceptCtrlNewline
		};

		TextEntry();
		virtual ~TextEntry();
		virtual void GetSizeRequested(float size[2]);
		virtual void Draw();
		virtual bool OnMouseDown(MouseButtonEvent *e);
		void SetText(const std::string &text);
		std::string GetText() const { return m_text; }
		void SetCursorPos(int pos) { m_cursPos = Clamp(pos, 0, signed(m_text.size())); }
		int GetCursorPos() const { return m_cursPos; };
		virtual bool OnKeyPress(const SDL_keysym *);
		virtual void Show() { GrabFocus(); Widget::Show(); }
		virtual void GrabFocus();
		void Unfocus();
		NewlineMode GetNewlineMode() const { return m_newlineMode; }
		void SetNewlineMode(NewlineMode mode) { m_newlineMode = mode; }

		// XXX probably a bad idea to use a signal with a return type
		// (by default, the return value will be whatever is returned by the
		//  last functor that's called when emitting the signal...
		//  you can provide your own 'accumulator' type though, to combine
		//  return values in different ways)
		sigc::signal<bool, const SDL_keysym*> onFilterKeys;
		sigc::signal<void, const SDL_keysym*> onKeyPress;
		sigc::signal<void> onValueChanged;
	private:

		void OnRawMouseDown(MouseButtonEvent *e);

		std::string m_text;
		int m_cursPos;
		int m_scroll;
		RefCountedPtr<Text::TextureFont> m_font;
		NewlineMode m_newlineMode;
		int m_newlineCount;

		bool m_justFocused;
		sigc::connection m_clickout;
	};
}

#endif /* _GUITEXTENTRY_H */
