#include "Gui.h"
#include "vector2.h"

namespace Gui {

Widget::Widget()
{
	m_parent = 0;
	m_enabled = true;
	m_visible = false;
	m_mouseOver = false;
	m_eventMask = EVENT_MOUSEMOTION;
	m_tooltipWidget = 0;
	m_shortcut.sym = SDLK_UNKNOWN;
	m_shortcut.mod = KMOD_NONE;
}

bool Widget::IsVisible() const
{
	if (!m_visible) return false;
	Container *parent = m_parent;
	while ((parent) && (parent->m_parent)) {
		if (parent->m_visible == false) return false;
		parent = parent->m_parent;
	}
	if (Screen::IsBaseWidget(parent))
		return parent->m_visible;
	else
		return false;
}

void Widget::SetScissor(bool enabled)
{
	if (enabled) {
		float pos[2];
		GetAbsolutePosition(pos);
		float scale[2];
		Gui::Screen::GetCoords2Pixels(scale);
		Gui::Screen::GetRenderer()->SetScissor(true, vector2f(pos[0]/scale[0],(float(Gui::Screen::GetHeight())-(pos[1]+m_size.h))/scale[1]), vector2f(m_size.w/scale[0],m_size.h/scale[1]));
	}
	else
		Gui::Screen::GetRenderer()->SetScissor(false);
}

void Widget::GrabFocus()
{
	Screen::SetFocused(this);
}

bool Widget::IsFocused()
{
	return Screen::IsFocused(this);
}

void Widget::SetShortcut(SDLKey key, SDLMod mod)
{
	assert(m_shortcut.sym == 0); // because AddShortcutWidget will add more than once. fix this otherwise on destruct we leave bad pointers in the Screen shortcut widgets list
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

void Widget::GetAbsolutePosition(float pos[2]) const
{
	const Container *parent = GetParent();

	if (parent) {
		float parentPos[2];
		parent->GetAbsolutePosition(parentPos);
		parent->GetChildPosition(this, pos);
		pos[0] += parentPos[0];
		pos[1] += parentPos[1];
	} else {
		pos[0] = pos[1] = 0;
	}
}
	
void Widget::OnMouseEnter()
{
	m_mouseOver = true;
	m_tooltipTimerConnection = Gui::AddTimer(1000, sigc::mem_fun(this, &Widget::OnToolTip));
	onMouseEnter.emit();
}

void Widget::OnMouseLeave()
{
	m_mouseOver = false;
	if (m_tooltipWidget) {
		Screen::RemoveBaseWidget(m_tooltipWidget);
		delete m_tooltipWidget;
		m_tooltipWidget = 0;
	}
	m_tooltipTimerConnection.disconnect();
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
	if (! IsVisible()) return;

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

		Screen::AddBaseWidget(m_tooltipWidget, int(pos[0]), int(pos[1]));
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
	m_tooltipTimerConnection.disconnect();
}

void Widget::ResizeRequest()
{
	if (!IsVisible()) return;
	if (m_parent) m_parent->OnChildResizeRequest(this);
	else {
		float size[2] = { FLT_MAX, FLT_MAX };
		GetSizeRequested(size);
		SetSize(size[0], size[1]);
	}
}

Widget::~Widget()
{
	onDelete.emit();
	if (m_tooltipWidget) {
		Screen::RemoveBaseWidget(m_tooltipWidget);
		delete m_tooltipWidget;
	}
	Screen::RemoveShortcutWidget(this);
	m_tooltipTimerConnection.disconnect();
}

}
