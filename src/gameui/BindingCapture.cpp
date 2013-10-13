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

KeyBindingCapture::~KeyBindingCapture()
{
	Disconnect();
}

void KeyBindingCapture::Capture()
{
	if (!m_capturing) {
		m_capturing = true;
		if (IsVisible()) {
			Connect();
		}
	}
}

void KeyBindingCapture::HandleVisible()
{
	if (m_capturing) {
		Connect();
	}
}

void KeyBindingCapture::HandleInvisible()
{
	if (m_capturing) {
		Disconnect();
	}
}

void KeyBindingCapture::HandleKeyDown(const UI::KeyboardEvent &event)
{
	if (m_capturing) {
		m_binding.type = KeyBindings::KEYBOARD_KEY;
		m_binding.u.keyboard.key = event.keysym.sym;
		m_binding.u.keyboard.mod = event.keysym.mod;
		m_capturing = false;
		Disconnect();
		onCapture.emit(m_binding);
	}
}

void KeyBindingCapture::Connect()
{
	assert(m_capturing);
	assert(IsVisible());
	GetContext()->SelectWidget(this);
	m_connJoystickAxisMove = GetContext()->onJoystickAxisMove.connect(sigc::mem_fun(this, &KeyBindingCapture::OnJoystickAxisMove));
	m_connJoystickHatMove = GetContext()->onJoystickHatMove.connect(sigc::mem_fun(this, &KeyBindingCapture::OnJoystickHatMove));
	m_connJoystickButtonDown = GetContext()->onJoystickButtonDown.connect(sigc::mem_fun(this, &KeyBindingCapture::OnJoystickButtonDown));
}

void KeyBindingCapture::Disconnect()
{
	m_connJoystickAxisMove.disconnect();
	m_connJoystickHatMove.disconnect();
	m_connJoystickButtonDown.disconnect();
}

bool KeyBindingCapture::OnJoystickAxisMove(const UI::JoystickAxisMotionEvent &event)
{
	// capture axis binding
	return false;
}

bool KeyBindingCapture::OnJoystickHatMove(const UI::JoystickHatMotionEvent &event)
{
	if (m_capturing) {
		m_binding.type = KeyBindings::JOYSTICK_HAT;
		m_binding.u.joystickHat.joystick = event.joystick;
		m_binding.u.joystickHat.hat = event.hat;
		m_binding.u.joystickHat.direction = static_cast<int>(event.direction);
		m_capturing = false;
		Disconnect();
		onCapture.emit(m_binding);
		return true;
	} else
		return false;
}

bool KeyBindingCapture::OnJoystickButtonDown(const UI::JoystickButtonEvent &event)
{
	if (m_capturing) {
		m_binding.type = KeyBindings::JOYSTICK_BUTTON;
		m_binding.u.joystickButton.joystick = event.joystick;
		m_binding.u.joystickButton.button = event.button;
		m_capturing = false;
		Disconnect();
		onCapture.emit(m_binding);
		return true;
	} else
		return false;
}

}
