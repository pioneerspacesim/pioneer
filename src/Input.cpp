// Copyright © 2008-2020 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#include "Input.h"
#include "GameConfig.h"
#include "InputBindings.h"
#include "Pi.h"
#include "SDL.h"
#include "SDL_events.h"
#include "SDL_joystick.h"
#include "ui/Context.h"

#include <array>
#include <sstream>
#include <type_traits>

using namespace Input;

namespace Input {
	std::vector<sigc::slot<void(Input::Manager *)>> *m_registrations;

	std::vector<sigc::slot<void(Input::Manager *)>> &GetBindingRegistration()
	{
		return *m_registrations;
	}

	bool AddBindingRegistrar(sigc::slot<void(Input::Manager *)> &&fn)
	{
		static std::vector<sigc::slot<void(Input::Manager *)>> registrations;
		m_registrations = &registrations;

		registrations.push_back(fn);
		return true;
	}
} // namespace Input

/*

	STATIC JOYSTICK HANDLING

*/

namespace Input {
	std::map<SDL_JoystickID, JoystickInfo> m_joysticks;

	InputBindings::Action nullAction;
	InputBindings::Axis nullAxis;
} // namespace Input

std::string Input::JoystickName(int joystick)
{
	return std::string(SDL_JoystickName(m_joysticks[joystick].joystick));
}

std::string Input::JoystickGUIDString(int joystick)
{
	const int guidBufferLen = 33; // as documented by SDL
	char guidBuffer[guidBufferLen];

	SDL_JoystickGetGUIDString(m_joysticks[joystick].guid, guidBuffer, guidBufferLen);
	return std::string(guidBuffer);
}

// conveniance version of JoystickFromGUID below that handles the string mangling.
int Input::JoystickFromGUIDString(const std::string &guid)
{
	return JoystickFromGUIDString(guid.c_str());
}

// conveniance version of JoystickFromGUID below that handles the string mangling.
int Input::JoystickFromGUIDString(const char *guid)
{
	return JoystickFromGUID(SDL_JoystickGetGUIDFromString(guid));
}

// return the internal ID of the stated joystick guid.
// returns -1 if we couldn't find the joystick in question.
int Input::JoystickFromGUID(SDL_JoystickGUID guid)
{
	const int guidLength = 16; // as defined
	for (auto pair : m_joysticks) {
		JoystickInfo &state = pair.second;
		if (0 == memcmp(state.guid.data, guid.data, guidLength)) {
			return static_cast<int>(pair.first);
		}
	}
	return -1;
}

SDL_JoystickGUID Input::JoystickGUID(int joystick)
{
	return m_joysticks[joystick].guid;
}

void Input::InitJoysticks()
{
	SDL_Init(SDL_INIT_JOYSTICK);

	int joy_count = SDL_NumJoysticks();
	Output("Initializing joystick subsystem.\n");
	for (int n = 0; n < joy_count; n++) {
		JoystickInfo state;

		state.joystick = SDL_JoystickOpen(n);
		if (!state.joystick) {
			Warning("SDL_JoystickOpen(%i): %s\n", n, SDL_GetError());
			continue;
		}

		state.guid = SDL_JoystickGetGUID(state.joystick);
		state.axes.resize(SDL_JoystickNumAxes(state.joystick));
		state.buttons.resize(SDL_JoystickNumButtons(state.joystick));
		state.hats.resize(SDL_JoystickNumHats(state.joystick));

		std::array<char, 33> joystickGUIDName;
		SDL_JoystickGetGUIDString(state.guid, joystickGUIDName.data(), joystickGUIDName.size());
		Output("Found joystick '%s' (GUID: %s)\n", SDL_JoystickName(state.joystick), joystickGUIDName.data());
		Output("  - %ld axes, %ld buttons, %ld hats\n", state.axes.size(), state.buttons.size(), state.hats.size());

		SDL_JoystickID joyID = SDL_JoystickInstanceID(state.joystick);
		m_joysticks[joyID] = state;
	}
}

std::map<SDL_JoystickID, JoystickInfo> &Input::GetJoysticks()
{
	return m_joysticks;
}

/*

	INPUT MANAGER INITIALIZATION

*/

Manager::Manager(IniConfig *config) :
	m_config(config),
	keyModState(0),
	mouseButton(),
	mouseMotion(),
	m_capturingMouse(false),
	joystickEnabled(true),
	mouseYInvert(false),
	m_enableBindings(true)
{
	joystickEnabled = (m_config->Int("EnableJoystick")) ? true : false;
	mouseYInvert = (m_config->Int("InvertMouseY")) ? true : false;

	Input::InitJoysticks();
}

void Manager::InitGame()
{
	//reset input states
	keyState.clear();
	keyModState = 0;
	mouseButton.fill(0);
	mouseMotion.fill(0);

	// Force a rebuild of key chords and modifier state
	m_frameListChanged = true;

	for (auto &pair : Input::GetJoysticks()) {
		JoystickInfo &state = pair.second;
		std::fill(state.buttons.begin(), state.buttons.end(), false);
		std::fill(state.hats.begin(), state.hats.end(), 0);
		std::fill(state.axes.begin(), state.axes.end(), JoystickInfo::Axis{ 0.0f, 0.0f });
	}
}

/*

	BINDING AND INPUT FRAME HANDLING

*/

InputBindings::Action *InputFrame::AddAction(std::string id)
{
	auto *action = manager->GetActionBinding(id);
	if (!action)
		throw std::runtime_error("Adding unknown action binding to InputFrame, id: " + id);

	actions.push_back(action);
	return action;
}

InputBindings::Axis *InputFrame::AddAxis(std::string id)
{
	auto *axis = manager->GetAxisBinding(id);
	if (!axis)
		throw std::runtime_error("Adding unknown axis binding to an InputFrame, id: " + id);

	axes.push_back(axis);
	return axis;
}

bool Manager::PushInputFrame(InputFrame *frame)
{
	if (HasInputFrame(frame)) {
		return false;
	}

	m_inputFrames.push_back(frame);
	frame->active = true;
	frame->onFrameAdded.emit(frame);
	m_frameListChanged = true;

	return true;
}

InputFrame *Manager::PopInputFrame()
{
	if (m_inputFrames.size() > 0) {
		auto frame = m_inputFrames.back();
		m_inputFrames.pop_back();
		frame->active = false;
		frame->onFrameRemoved.emit(frame);
		m_frameListChanged = true;

		return frame;
	}

	return nullptr;
}

void Manager::RemoveInputFrame(InputFrame *frame)
{
	auto it = std::find(m_inputFrames.begin(), m_inputFrames.end(), frame);
	if (it != m_inputFrames.end()) {
		m_inputFrames.erase(it);

		// When an input frame is removed, its actions and axes are no longer active.
		for (auto *action : frame->actions) {
			if (action->m_active) {
				action->m_active = false;
				action->onReleased.emit();
			}
		}

		for (auto *axis : frame->axes) {
			if (axis->m_value != 0.0) {
				axis->m_value = 0.0;
				axis->onAxisValue.emit(0.0);
			}
		}

		frame->active = false;
		frame->onFrameRemoved.emit(frame);
		m_frameListChanged = true;
	}
}

InputBindings::Action *Manager::AddActionBinding(std::string id, BindingGroup *group, InputBindings::Action &&binding)
{
	// throw an error if we attempt to bind an action onto an already-bound axis in the same group.
	if (group->bindings.count(id) && group->bindings[id] != BindingGroup::ENTRY_ACTION)
		Error("Attempt to bind already-registered axis %s as an action.\n", id.c_str());

	group->bindings[id] = BindingGroup::ENTRY_ACTION;

	// Load from the config
	std::string config_str = m_config->String(id.c_str());
	if (!config_str.empty()) {
		nonstd::string_view str(config_str);
		str >> binding;
	}

	return &(actionBindings[id] = binding);
}

InputBindings::Axis *Manager::AddAxisBinding(std::string id, BindingGroup *group, InputBindings::Axis &&binding)
{
	// throw an error if we attempt to bind an axis onto an already-bound action in the same group.
	if (group->bindings.count(id) && group->bindings[id] != BindingGroup::ENTRY_AXIS)
		Error("Attempt to bind already-registered action %s as an axis.\n", id.c_str());

	group->bindings[id] = BindingGroup::ENTRY_AXIS;

	// Load from the config
	std::string config_str = m_config->String(id.c_str());
	if (!config_str.empty()) {
		nonstd::string_view str(config_str);
		str >> binding;
	}

	return &(axisBindings[id] = binding);
}

InputBindings::Action *Manager::GetActionBinding(std::string id)
{
	return actionBindings.count(id) ? &actionBindings[id] : &Input::nullAction;
}

InputBindings::Axis *Manager::GetAxisBinding(std::string id)
{
	return axisBindings.count(id) ? &axisBindings[id] : &Input::nullAxis;
}

/*

	STATE MANAGEMENT

*/

bool Manager::GetBindingState(InputBindings::KeyBinding &key)
{
	using Type = InputBindings::KeyBinding::Type;

	switch (key.type) {
	case Type::Disabled:
		return false;
	case Type::KeyboardKey:
		return KeyState(key.keycode);
	case Type::JoystickButton:
		return JoystickButtonState(key.joystick.id, key.joystick.button);
	case Type::JoystickHat:
		return (JoystickHatState(key.joystick.id, key.joystick.hat) & key.joystick.button) == key.joystick.button;
	case Type::MouseButton:
		return MouseButtonState(key.mouse.button);
	default:
		return false;
	}
}

float Manager::GetAxisState(InputBindings::JoyAxis &axis)
{
	if (axis.direction == 0)
		return 0.0; // disabled

	return JoystickAxisState(axis.joystickId, axis.axis) * float(axis.direction);
}

bool Manager::GetModifierState(InputBindings::KeyChord *chord)
{
	bool mod1 = chord->modifier1.Enabled() ? m_modifiers[chord->modifier1] : true;
	bool mod2 = chord->modifier2.Enabled() ? m_modifiers[chord->modifier2] : true;
	return mod1 && mod2;
}

int Manager::JoystickButtonState(int joystick, int button)
{
	if (!joystickEnabled) return 0;
	if (joystick < 0 || joystick >= int(GetJoysticks().size()))
		return 0;

	if (button < 0 || button >= int(GetJoysticks()[joystick].buttons.size()))
		return 0;

	return GetJoysticks()[joystick].buttons[button];
}

int Manager::JoystickHatState(int joystick, int hat)
{
	if (!joystickEnabled) return 0;
	if (joystick < 0 || joystick >= int(GetJoysticks().size()))
		return 0;

	if (hat < 0 || hat >= int(GetJoysticks()[joystick].hats.size()))
		return 0;

	return GetJoysticks()[joystick].hats[hat];
}

float Manager::JoystickAxisState(int joystick, int axis)
{
	if (!joystickEnabled) return 0;
	if (joystick < 0 || joystick >= int(GetJoysticks().size()))
		return 0;

	if (axis < 0 || axis >= int(GetJoysticks()[joystick].axes.size()))
		return 0;

	return GetJoysticks()[joystick].axes[axis].value;
}

void Manager::SetJoystickEnabled(bool state)
{
	joystickEnabled = state;
	if (m_enableConfigSaving) {
		m_config->SetInt("EnableJoystick", joystickEnabled);
		m_config->Save();
	}
}

void Manager::SetMouseYInvert(bool state)
{
	mouseYInvert = state;
	if (m_enableConfigSaving) {
		m_config->SetInt("InvertMouseY", mouseYInvert);
		m_config->Save();
	}
}

/*

	FRAME AND EVENT HANDLING

*/

void Manager::NewFrame()
{
	mouseMotion.fill(0);
	mouseWheel = 0;
	for (auto &k : keyState) {
		auto &val = keyState[k.first];
		switch (k.second) {
		case 1: // if we were just pressed last frame, migrate to held state
			val = 2;
			break;
		case 4: // if we were just released last frame, migrate to empty state
			val = 0;
			break;
		default: // otherwise, no need to do anything
			break;
		}
	}

	if (m_frameListChanged) {
		RebuildInputFrames();
	}
}

void Manager::RebuildInputFrames()
{
	// Reset the list of active chords.
	m_chords.clear();
	m_activeActions.clear();
	m_activeAxes.clear();

	for (const auto *frame : reverse_container(m_inputFrames)) {

		// Push all enabled key chords onto the key chord stack.
		for (auto *action : frame->actions) {
			if (!action->Enabled())
				continue;

			m_activeActions.push_back(action);
			assert(m_activeActions.back() == action);
			if (action->binding.Enabled())
				m_chords.push_back(&action->binding);

			if (action->binding2.Enabled())
				m_chords.push_back(&action->binding2);
		}

		for (auto *axis : frame->axes) {
			if (!axis->Enabled())
				continue;

			m_activeAxes.push_back(axis);
			if (axis->positive.Enabled())
				m_chords.push_back(&axis->positive);

			if (axis->negative.Enabled())
				m_chords.push_back(&axis->negative);
		}

		// If we have a modal frame, it prevents input from passing through it to frames below
		if (frame->modal) { // modal frame blocks all inputs below it
			break;
		}
	}

	// Group all chords with the same number of modifiers together, in descending order.
	std::sort(m_chords.begin(), m_chords.end(), [](const InputBindings::KeyChord *a, const InputBindings::KeyChord *b) { return *a < *b; });

	// Reinitialize the modifier list, preserving key state.
	m_modifiers.clear();
	for (auto *chord : m_chords) {
		m_modifiers.emplace(chord->modifier1, GetBindingState(chord->modifier1));
		m_modifiers.emplace(chord->modifier2, GetBindingState(chord->modifier2));
	}
}

static int8_t keys_in_chord(InputBindings::KeyChord *chord)
{
	return chord->activator.Enabled() + chord->modifier1.Enabled() + chord->modifier2.Enabled();
}

void Manager::HandleSDLEvent(SDL_Event &event)
{
	using namespace InputBindings;

	switch (event.type) {
	case SDL_KEYDOWN:
		// Set key state to "just pressed"
		keyState[event.key.keysym.sym] = 1;
		keyModState = event.key.keysym.mod;
		onKeyPress.emit(&event.key.keysym);
		break;
	case SDL_KEYUP:
		// Set key state to "just released"
		keyState[event.key.keysym.sym] = 4;
		keyModState = event.key.keysym.mod;
		onKeyRelease.emit(&event.key.keysym);
		break;
	case SDL_MOUSEBUTTONDOWN:
		if (event.button.button < mouseButton.size()) {
			mouseButton[event.button.button] = 1;
			onMouseButtonDown.emit(event.button.button,
				event.button.x, event.button.y);
		}
		break;
	case SDL_MOUSEBUTTONUP:
		if (event.button.button < mouseButton.size()) {
			mouseButton[event.button.button] = 0;
			onMouseButtonUp.emit(event.button.button,
				event.button.x, event.button.y);
		}
		break;
	case SDL_MOUSEWHEEL:
		mouseWheel = event.wheel.y;
		onMouseWheel.emit(event.wheel.y > 0); // true = up
		break;
	case SDL_MOUSEMOTION:
		mouseMotion[0] += event.motion.xrel;
		mouseMotion[1] += event.motion.yrel;
		break;
	case SDL_JOYAXISMOTION:
		if (!GetJoysticks()[event.jaxis.which].joystick)
			break;
		if (event.jaxis.value == -32768)
			GetJoysticks()[event.jaxis.which].axes[event.jaxis.axis].value = 1.f;
		else
			GetJoysticks()[event.jaxis.which].axes[event.jaxis.axis].value = -event.jaxis.value / 32767.f;
		break;
	case SDL_JOYBUTTONUP:
	case SDL_JOYBUTTONDOWN:
		if (!GetJoysticks()[event.jaxis.which].joystick)
			break;
		GetJoysticks()[event.jbutton.which].buttons[event.jbutton.button] = event.jbutton.state != 0;
		break;
	case SDL_JOYHATMOTION:
		if (!GetJoysticks()[event.jaxis.which].joystick)
			break;
		GetJoysticks()[event.jhat.which].hats[event.jhat.hat] = event.jhat.value;
		break;
	default:
		// Don't process non-input events any further.
		return;
	}

	// if bindings are disabled, don't process the event any further
	if (!m_enableBindings)
		return;

	// Update the modifier status from this event
	for (auto &pair : m_modifiers) {
		auto r = pair.first.Matches(event);
		if (r != Response::Ignored) {
			pair.second = r == Response::Pressed ? true : false;
		}
	}

	// If the event matches one of the key chords we care about, update that chord
	int num_keys_in_chord = 0;
	for (auto *chord : m_chords) {
		Response activator = chord->activator.Matches(event);
		if (activator == Response::Ignored)
			continue;

		if (chord->IsActive()) {
			// Another press event came in for a key that's currently pressed right now.
			// This should be sufficiently rare that it won't be happening.
			if (activator == Response::Pressed)
				break;
			else { // clear the active state, continue processing the release event
				chord->m_active = false;
				// in case of a press-release sequence in the same frame, make sure to properly send updates.
				chord->m_queuedEvents |= 2;
			}
		} else {
			// Key-release event for a non-active chord. Don't handle it, but pass it on so
			// another key chord (with fewer / different modifiers) can handle it.
			if (activator == Response::Released)
				continue;

			// Break here to prevent CTRL+ALT+X from activating <CTRL>+X / <ALT>+X / <NONE>+X
			// when there's a CTRL+ALT+X binding
			if (keys_in_chord(chord) < num_keys_in_chord)
				break;

			bool mod1 = chord->modifier1.Enabled() ? m_modifiers[chord->modifier1] : true;
			bool mod2 = chord->modifier2.Enabled() ? m_modifiers[chord->modifier2] : true;
			if (mod1 && mod2) {
				// Modifiers are pressed, we can activate the chord.
				chord->m_active = true;
				// in the case of a press-release in the same frame, make sure to properly send updates
				chord->m_queuedEvents |= 1;
				// all copies of this chord should be notified, but don't propagate to chords with fewer modifiers
				num_keys_in_chord = keys_in_chord(chord);
			}
		}
	}
}

void Manager::DispatchEvents()
{
	// Chords which have had their modifier keys released this frame get updated all at once
	for (auto *chord : m_chords) {
		if (chord->IsActive() && !GetModifierState(chord)) {
			chord->m_active = false;
			chord->m_queuedEvents |= 2;
		}
	}

	for (auto *action : m_activeActions) {
		// if we have queued events for this binding, make sure to
		uint8_t queued = action->binding.m_queuedEvents | action->binding2.m_queuedEvents;
		if (queued) {
			bool wasActive = action->m_active;
			action->m_active = action->binding.IsActive() || action->binding2.IsActive();

			// if at least one of the bindings was pressed this frame and the action was not
			// previously active, call the pressed event
			if (queued & 1 && !wasActive)
				action->onPressed.emit();

			// if at least one of the bindings was released this frame but are not pressed currently,
			// call the released event
			if (queued & 2 && !action->m_active)
				action->onReleased.emit();

			// clear queued events
			action->binding.m_queuedEvents = action->binding2.m_queuedEvents = 0;
		}
	}

	for (auto *axis : m_activeAxes) {
		float value = GetAxisState(axis->axis);

		value += axis->positive.IsActive();
		value -= axis->negative.IsActive();

		value = Clamp(value, -1.0f, 1.0f);
		if (value != 0.0 || axis->m_value != 0.0) {
			axis->m_value = value;
			axis->onAxisValue.emit(value);
		}
	}
}

/*

	INPUT DEVICE MANAGEMENT ROUTINES

*/

void Manager::SetCapturingMouse(bool grabbed)
{
	// early-out to avoid changing (possibly) expensive WM state
	if (grabbed == m_capturingMouse)
		return;

	SDL_SetWindowGrab(Pi::renderer->GetSDLWindow(), SDL_bool(grabbed));
	SDL_SetRelativeMouseMode(SDL_bool(grabbed));
	m_capturingMouse = grabbed;
}
