#include "Gui.h"
#include "GuiRepeaterButton.h"

namespace Gui {

RepeaterButton::RepeaterButton(int msDelay, int msRepeat)
{
	m_delay = msDelay;
	m_repeat = msRepeat;

	onPress.connect(sigc::mem_fun(this, &RepeaterButton::OnPress));
	onRelease.connect(sigc::mem_fun(this, &RepeaterButton::OnRelease));
	m_repeatsig.connect(sigc::mem_fun(this, &RepeaterButton::OnRepeat));
}

RepeaterButton::~RepeaterButton()
{
	Gui::RemoveTimer(&m_repeatsig);
}

void RepeaterButton::OnPress()
{
	Gui::AddTimer(m_delay, &m_repeatsig);
}

void RepeaterButton::OnRelease()
{
	Gui::RemoveTimer(&m_repeatsig);
}

void RepeaterButton::OnRepeat()
{
	Gui::AddTimer(m_repeat, &m_repeatsig);

	onClick.emit();
}

} /* namespace Gui */
