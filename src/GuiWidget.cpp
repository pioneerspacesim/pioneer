#include "Gui.h"

namespace Gui {

Widget::Widget()
{
	m_parent = 0;
	m_visible = false;
	m_mouseOver = false;
	m_eventMask = EVENT_NONE;
	m_tooltipWidget = 0;
	m_tooltipTimerSignal.connect(sigc::mem_fun(this, &Widget::OnToolTip));
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

void Widget::GetAbsolutePosition(float pos[2])
{
	GetPosition(pos);
	const Container *parent = GetParent();
	while (parent) {
		pos[0] += parent->m_size.x;
		pos[1] += parent->m_size.y;
		parent = parent->GetParent();
	}
}
	
void Widget::OnMouseEnter()
{
	m_mouseOver = true;
	Gui::AddTimer(1000, &m_tooltipTimerSignal);
	onMouseEnter.emit();
}

void Widget::OnMouseLeave()
{
	m_mouseOver = false;
	if (m_tooltipWidget) {
		Screen::RemoveBaseWidget(m_tooltipWidget);
		m_tooltipWidget = 0;
	}
	Gui::RemoveTimer(&m_tooltipTimerSignal);
	onMouseLeave.emit();
}

void Widget::UpdateOverriddenTooltip()
{
	if (m_tooltipWidget) {
		std::string text = GetOverrideTooltip();
		m_tooltipWidget->SetText(text);
	}
}

void Widget::OnToolTip()
{
	if (!m_tooltipWidget) {
		std::string text = GetOverrideTooltip();
		if (text == "") text = m_tooltip;
		if (text == "") return;

		float pos[2];
		GetAbsolutePosition(pos);
		m_tooltipWidget = new ToolTip(text);
		if (m_tooltipWidget->m_size.w + pos[0] > Screen::GetWidth())
			pos[0] = Screen::GetWidth() - m_tooltipWidget->m_size.w;
		if (m_tooltipWidget->m_size.h + pos[1] > Screen::GetHeight())
			pos[1] = Screen::GetHeight() - m_tooltipWidget->m_size.h;

		Screen::AddBaseWidget(m_tooltipWidget, pos[0], pos[1]);
		m_tooltipWidget->Show();
	}
}

void Widget::Hide()
{
	m_visible = false;
	if (m_tooltipWidget) {
		Screen::RemoveBaseWidget(m_tooltipWidget);
		delete m_tooltipWidget;
		m_tooltipWidget = 0;
	}
}

Widget::~Widget()
{
	if (m_tooltipWidget) {
		Screen::RemoveBaseWidget(m_tooltipWidget);
		delete m_tooltipWidget;
	}
}

}
