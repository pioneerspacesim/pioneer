// Copyright © 2008-2013 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#include "Gui.h"
#include "GuiRepeaterButton.h"

namespace Gui {

RepeaterButton::RepeaterButton(int msDelay, int msRepeat)
{
	m_delay = msDelay;
	m_repeat = msRepeat;

	onPress.connect(sigc::mem_fun(this, &RepeaterButton::OnPress));
	onRelease.connect(sigc::mem_fun(this, &RepeaterButton::OnRelease));
}

RepeaterButton::~RepeaterButton()
{
	m_repeatCon.disconnect();
}

void RepeaterButton::OnPress()
{
	m_repeatCon = Gui::AddTimer(m_delay, sigc::mem_fun(this, &RepeaterButton::OnRepeat));
}

void RepeaterButton::OnRelease()
{
	m_repeatCon.disconnect();
}

void RepeaterButton::OnRepeat()
{
	m_repeatCon = Gui::AddTimer(m_repeat, sigc::mem_fun(this, &RepeaterButton::OnRepeat));

	onClick.emit();
}

} /* namespace Gui */
