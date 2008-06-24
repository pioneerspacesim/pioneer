#include "Gui.h"

namespace Gui {

Widget::Widget()
{
	m_visible = false;
	m_eventMask = EVENT_NONE;
}

void Widget::SetShortcut(SDLKey key, SDLMod mod)
{
	m_shortcut.sym = key;
	m_shortcut.mod = mod;
	Screen::AddShortcutWidget(this);
}

void Widget::OnPreShortcut(const SDL_keysym *sym)
{
	int mod = sym->mod & 0xfff; // filters out numlock, capslock, which fuck things up
	if ((sym->sym == m_shortcut.sym) && (mod == m_shortcut.mod)) {
		OnActivate();
	}
}

}
