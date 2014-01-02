// Copyright © 2008-2014 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#include "BindingCapture.h"

using namespace UI;

namespace GameUI {

KeyBindingCapture::KeyBindingCapture(UI::Context *context): Single(context)
{
	m_binding.type = KeyBindings::KEYBOARD_KEY;
	m_binding.u.keyboard.key = SDLK_UNKNOWN;
	m_binding.u.keyboard.mod = KMOD_NONE;
}

KeyBindingCapture::~KeyBindingCapture()
{
	Disconnect();
}

void KeyBindingCapture::HandleVisible()
{
	Connect();
}

void KeyBindingCapture::HandleInvisible()
{
	Disconnect();
}

void KeyBindingCapture::HandleKeyDown(const UI::KeyboardEvent &event)
{
	m_binding.type = KeyBindings::KEYBOARD_KEY;
	m_binding.u.keyboard.key = event.keysym.sym;
	m_binding.u.keyboard.mod = event.keysym.mod;
	Disconnect();
	onCapture.emit(m_binding);
}

void KeyBindingCapture::Connect()
{
	assert(IsVisible());
	GetContext()->SelectWidget(this);
	m_connJoystickHatMove = GetContext()->onJoystickHatMove.connect(sigc::mem_fun(this, &KeyBindingCapture::OnJoystickHatMove));
	m_connJoystickButtonDown = GetContext()->onJoystickButtonDown.connect(sigc::mem_fun(this, &KeyBindingCapture::OnJoystickButtonDown));
}

void KeyBindingCapture::Disconnect()
{
	m_connJoystickHatMove.disconnect();
	m_connJoystickButtonDown.disconnect();
}

bool KeyBindingCapture::OnJoystickHatMove(const UI::JoystickHatMotionEvent &event)
{
	m_binding.type = KeyBindings::JOYSTICK_HAT;
	m_binding.u.joystickHat.joystick = event.joystick;
	m_binding.u.joystickHat.hat = event.hat;
	m_binding.u.joystickHat.direction = static_cast<int>(event.direction);
	Disconnect();
	onCapture.emit(m_binding);
	return true;
}

bool KeyBindingCapture::OnJoystickButtonDown(const UI::JoystickButtonEvent &event)
{
	m_binding.type = KeyBindings::JOYSTICK_BUTTON;
	m_binding.u.joystickButton.joystick = event.joystick;
	m_binding.u.joystickButton.button = event.button;
	Disconnect();
	onCapture.emit(m_binding);
	return true;
}

AxisBindingCapture::AxisBindingCapture(UI::Context *context): Single(context)
{
	m_binding.joystick = 0;
	m_binding.axis = 0;
	m_binding.direction = KeyBindings::POSITIVE;
}

AxisBindingCapture::~AxisBindingCapture()
{
	Disconnect();
}

void AxisBindingCapture::HandleVisible()
{
	Connect();
}

void AxisBindingCapture::HandleInvisible()
{
	Disconnect();
}

void AxisBindingCapture::Connect()
{
	assert(IsVisible());
	m_connJoystickAxisMove = GetContext()->onJoystickAxisMove.connect(sigc::mem_fun(this, &AxisBindingCapture::OnJoystickAxisMove));
}

void AxisBindingCapture::Disconnect()
{
	m_connJoystickAxisMove.disconnect();
}

bool AxisBindingCapture::OnJoystickAxisMove(const UI::JoystickAxisMotionEvent &event)
{
	const float threshold = 0.4f; // joystick axis value is in range -1 to 1
	if (event.value < -threshold || event.value > threshold) {
		m_binding.joystick = event.joystick;
		m_binding.axis = event.axis;
		m_binding.direction = (event.value > 0 ? KeyBindings::POSITIVE : KeyBindings::NEGATIVE);
		Disconnect();
		onCapture.emit(m_binding);
		return true;
	}
	return false;
}

}
