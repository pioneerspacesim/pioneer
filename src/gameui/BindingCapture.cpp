// Copyright © 2008-2013 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#include "BindingCapture.h"

using namespace UI;

namespace GameUI {

KeyBindingCapture::KeyBindingCapture(UI::Context *context): Single(context), m_capturing(false)
{
	m_binding.type = KeyBindings::KEYBOARD_KEY;
	m_binding.u.keyboard.key = SDLK_UNKNOWN;
	m_binding.u.keyboard.mod = KMOD_NONE;
}

void KeyBindingCapture::Capture()
{
	GetContext()->SelectWidget(this);
	m_capturing = true;
}

void KeyBindingCapture::HandleKeyDown(const UI::KeyboardEvent &event)
{
	if (m_capturing) {
		m_binding.type = KeyBindings::KEYBOARD_KEY;
		m_binding.u.keyboard.key = event.keysym.sym;
		m_binding.u.keyboard.mod = event.keysym.mod;
		m_capturing = false;
		onCapture.emit(m_binding);
	}
}

}
