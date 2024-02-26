// Copyright © 2008-2024 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#include "Input.h"
#include "AnimationCurves.h"
#include "GameConfig.h"
#include "InputBindings.h"
#include "Pi.h"
#include "utils.h"

#include "SDL.h"
#include "SDL_events.h"
#include "SDL_joystick.h"
#include "SDL_mouse.h"

#include <algorithm>
#include <array>
#include <regex>
#include <sstream>
#include <type_traits>

using namespace Input;

namespace Input {
	std::vector<sigc::slot<void, Input::Manager *>> *m_registrations;

	std::vector<sigc::slot<void, Input::Manager *>> &GetBindingRegistration()
	{
		return *m_registrations;
	}

	bool AddBindingRegistrar(sigc::slot<void, Input::Manager *> &&fn)
	{
		static std::vector<sigc::slot<void, Input::Manager *>> registrations;
		m_registrations = &registrations;

		registrations.push_back(fn);
		return true;
	}
} // namespace Input

/*

	STATIC JOYSTICK HANDLING

*/

namespace Input {
	// mapping SDL instance ID of joystick to internal ID
	std::vector<uint32_t> m_joyIDs;
	std::vector<JoystickInfo> m_joysticks;

	InputBindings::Action nullAction;
	InputBindings::Axis nullAxis;
} // namespace Input

std::string Input::JoystickName(int joystick)
{
	return m_joysticks[joystick].name;
}

std::string Input::JoystickGUIDString(int joystick)
{
	const int guidBufferLen = 33; // as documented by SDL
	char guidBuffer[guidBufferLen];

	SDL_JoystickGetGUIDString(m_joysticks[joystick].guid, guidBuffer, guidBufferLen);
	return std::string(guidBuffer);
}

// convenience version of JoystickFromGUID below that handles the string mangling.
int Input::JoystickFromGUIDString(const std::string &guid)
{
	return JoystickFromGUIDString(guid.c_str());
}

// convenience version of JoystickFromGUID below that handles the string mangling.
int Input::JoystickFromGUIDString(const char *guid)
{
	return JoystickFromGUID(SDL_JoystickGetGUIDFromString(guid));
}

// return the internal ID of the stated joystick guid.
// returns -1 if we couldn't find the joystick in question.
int Input::JoystickFromGUID(SDL_JoystickGUID guid)
{
	const int guidLength = 16; // as defined
	for (unsigned i = 0; i < m_joysticks.size(); ++i) {
		auto &state = m_joysticks[i];
		if (0 == memcmp(state.guid.data, guid.data, guidLength)) {
			return i;
		}
	}
	return -1;
}

int Input::JoystickFromID(SDL_JoystickID id)
{
	assert(id >= 0 && "this ID means no joystick");
	assert(static_cast<unsigned>(id) < m_joyIDs.size() && "there is no internal ID for this SDL ID");
	return m_joyIDs[id];
}

SDL_JoystickGUID Input::JoystickGUID(int joystick)
{
	return m_joysticks[joystick].guid;
}

static std::string saveAxisConfig(const Input::JoystickInfo::Axis &axis)
{
	return fmt::format("DZ{:.2f} CV{:.2f}{}", axis.deadzone, axis.curve, (axis.zeroToOne ? " Half" : ""));
}

static void loadAxisConfig(const std::string &str, Input::JoystickInfo::Axis &outAxis)
{
	std::regex matcher("DZ([\\d\\.]+)\\s*(?:CV(-?[\\d\\.]+))?\\s*(Half)?", std::regex::icase);
	std::smatch match_results;
	if (std::regex_search(str, match_results, matcher)) {
		outAxis.deadzone = std::stof(match_results[1].str());
		outAxis.curve = match_results[2].matched ? std::stof(match_results[2].str()) : 1.0;
		outAxis.zeroToOne = match_results[3].matched;
	}
	outAxis.value = 0;
}

static JoystickInfo loadJoystick(SDL_Joystick *joystick, const IniConfig *config)
{
	JoystickInfo state;
	state.joystick = joystick;
	state.name = SDL_JoystickName(state.joystick);
	state.guid = SDL_JoystickGetGUID(state.joystick);
	state.axes.resize(SDL_JoystickNumAxes(state.joystick));
	state.buttons.resize(SDL_JoystickNumButtons(state.joystick));
	state.hats.resize(SDL_JoystickNumHats(state.joystick));

	std::array<char, 33> joystickGUIDName;
	SDL_JoystickGetGUIDString(state.guid, joystickGUIDName.data(), joystickGUIDName.size());
	Log::Info("Found joystick '{}' (GUID: {})\n", SDL_JoystickName(state.joystick), joystickGUIDName.data());
	Log::Info("  - {} axes, {} buttons, {} hats\n", state.axes.size(), state.buttons.size(), state.hats.size());

	std::string joystickName = "Joystick." + std::string(joystickGUIDName.data());

	for (size_t i = 0; i < state.axes.size(); i++) {
		std::string axisName = "Axis" + std::to_string(i);
		if (!config->HasEntry(joystickName, axisName)) {
			state.axes[i] = JoystickInfo::Axis {};
			continue;
		}

		loadAxisConfig(config->String(joystickName, axisName, ""), state.axes[i]);
		Log::Info("  - axis {}: deadzone {:.2f}, curve: {:.2f}, half-axis mode: {}\n",
			i, state.axes[i].deadzone, state.axes[i].curve, state.axes[i].zeroToOne);
	}
	return state;
}

void Input::SaveJoystickConfig(uint32_t joystick, IniConfig *config)
{
	if (joystick > GetJoysticks().size())
		return;

	JoystickInfo &state = GetJoysticks()[joystick];

	std::array<char, 33> joystickGUIDName;
	SDL_JoystickGetGUIDString(state.guid, joystickGUIDName.data(), joystickGUIDName.size());

	std::string joystickName = "Joystick." + std::string(joystickGUIDName.data());
	config->SetString(joystickName, "Name", state.name);

	for (size_t i = 0; i < state.axes.size(); i++) {
		std::string axisName = "Axis" + std::to_string(i);
		config->SetString(joystickName, axisName, saveAxisConfig(state.axes[i]));
	}
}

void Input::AddJoystickID(SDL_JoystickID sdl_id, uint32_t internal_id)
{
	assert(sdl_id < 1000 && "SDL IDs seem to be big enough to use vector as map");
	assert(sdl_id >= 0 && "attempt to add absent ID");
	if (m_joyIDs.size() <= static_cast<unsigned>(sdl_id))
		m_joyIDs.resize(sdl_id + 1);
	m_joyIDs[sdl_id] = internal_id;
}

void Input::InitJoysticks(IniConfig *config)
{
	PROFILE_SCOPED()
	SDL_Init(SDL_INIT_JOYSTICK);

	int joy_count = SDL_NumJoysticks();
	Output("Initializing joystick subsystem.\n");

	for (int n = 0; n < joy_count; n++) {

		SDL_Joystick *joystick = SDL_JoystickOpen(n);
		if (!joystick) {
			Warning("SDL_JoystickOpen(%i): %s\n", n, SDL_GetError());
			continue;
		}

		JoystickInfo state = loadJoystick(joystick, config);

		SDL_JoystickID instance_id = SDL_JoystickInstanceID(state.joystick);
		AddJoystickID(instance_id, m_joysticks.size());
		m_joysticks.push_back(state);
	}

	for (size_t joystick = 0; joystick < m_joysticks.size(); joystick++) {
		SaveJoystickConfig(joystick, config);
	}

	config->Save();

	// now let's look for joysticks that are in the config, but not connected
	for (auto &section : config->GetSections()) {
		if (section.first.find("Joystick.") != 0) continue;
		auto guid = section.first.substr(9);
		if (JoystickFromGUIDString(guid.c_str()) >= 0) continue; // already connected
		// create unconnected joystick
		JoystickInfo state;
		state.joystick = nullptr;
		state.name = section.second["Name"];
		state.guid = SDL_JoystickGetGUIDFromString(guid.c_str());
		state.axes.resize(section.second.size());
		for (auto &param : section.second) {
			if (param.first.find("Axis") != 0) continue;
			int i = std::stoi(param.first.substr(4));
			loadAxisConfig(param.second, state.axes[i]);
		}
		m_joysticks.push_back(state);
	}
}

std::vector<JoystickInfo> &Input::GetJoysticks()
{
	return m_joysticks;
}

/*

	INPUT MANAGER INITIALIZATION

*/

Manager::Manager(IniConfig *config, SDL_Window *window) :
	m_window(window),
	m_config(config),
	keyModState(0),
	mouseButton(),
	mouseMotion(),
	m_capturingMouse(false),
	joystickEnabled(true),
	mouseYInvert(false),
	m_enableBindings(true),
	m_frameListChanged(false)
{
	joystickEnabled = (m_config->Int("EnableJoystick")) ? true : false;
	mouseYInvert = (m_config->Int("InvertMouseY")) ? true : false;

	Input::InitJoysticks(m_config);
}

void Manager::InitGame()
{
	//reset input states
	keyState.clear();
	keyModState = 0;
	ClearMouse();

	// Force a rebuild of key chords and modifier state
	m_frameListChanged = true;

	for (auto &state : Input::GetJoysticks()) {
		std::fill(state.buttons.begin(), state.buttons.end(), false);
		std::fill(state.hats.begin(), state.hats.end(), 0);
		for (auto &ax : state.axes) {
			ax.value = 0.0;
		}
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

bool Manager::AddInputFrame(InputFrame *frame)
{
	auto iter = std::find(m_inputFrames.begin(), m_inputFrames.end(), frame);
	if (iter != m_inputFrames.end()) {
		m_inputFrames.erase(iter);
	}

	m_inputFrames.push_back(frame);
	frame->active = true;
	frame->onFrameAdded.emit(frame);
	m_frameListChanged = true;

	return true;
}

// When an input frame is removed or masked by a modal frame,
// its actions and axes are no longer active and bindings need
// to be cleared to avoid orphaned state.
void ClearInputFrameState(InputFrame *frame)
{
	for (auto *action : frame->actions) {
		action->binding.m_active = false;
		action->binding.m_queuedEvents = 0;
		action->binding2.m_active = false;
		action->binding2.m_queuedEvents = 0;

		if (action->m_active) {
			action->m_active = false;
			action->onReleased.emit();
		}
	}

	for (auto *axis : frame->axes) {
		axis->negative.m_active = false;
		axis->negative.m_queuedEvents = false;
		axis->positive.m_active = false;
		axis->positive.m_queuedEvents = false;

		if (axis->m_value != 0.0) {
			axis->m_value = 0.0;
			axis->onAxisValue.emit(0.0);
		}
	}
}

bool Manager::HasInputFrame(InputFrame *frame)
{
	return std::count(m_inputFrames.begin(), m_inputFrames.end(), frame) > 0;
}

void Manager::RemoveInputFrame(InputFrame *frame)
{
	auto it = std::find(m_inputFrames.begin(), m_inputFrames.end(), frame);
	if (it != m_inputFrames.end()) {
		m_inputFrames.erase(it);

		ClearInputFrameState(frame);
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
		std::string_view str(config_str);
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
		std::string_view str(config_str);
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

void Manager::GetMousePosition(int position[2])
{
	SDL_GetMouseState(&position[0], &position[1]);
}

/*

	FRAME AND EVENT HANDLING

*/

void Manager::NewFrame()
{
	mouseMotion.fill(0);
	mouseWheel = 0;

	for (char &m : mouseButton) {
		if (m == 1) // if we were just pressed last frame, migrate to held state
			m = 2;
		if (m == 4) // if we were just released last frame, migrate to empty state
			m = 0;
	}

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

	bool hasModal = false;
	for (auto *frame : reverse_container(m_inputFrames)) {
		// Disable all frames that are masked by a modal frame
		// We reset all binding state here, because otherwise state is orphaned when events come in
		// while the modal is active and binding state no longer matches the actual hardware state
		// TODO: track when a frame is activated / deactivated and update state there?
		frame->active = !hasModal;
		if (hasModal) {
			ClearInputFrameState(frame);
			continue;
		}

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
			hasModal = true;
		}
	}

	// Group all chords with the same number of modifiers together, in descending order.
	std::sort(m_chords.begin(), m_chords.end(), [](const InputBindings::KeyChord *a, const InputBindings::KeyChord *b) { return *a < *b; });

	// Reinitialize the modifier list, preserving key state.
	m_modifiers.clear();
	for (auto *chord : m_chords) {
		if (chord->modifier1.Enabled())
			m_modifiers.emplace(chord->modifier1, GetBindingState(chord->modifier1));
		if (chord->modifier2.Enabled())
			m_modifiers.emplace(chord->modifier2, GetBindingState(chord->modifier2));
	}

	m_frameListChanged = false;
}

static int8_t keys_in_chord(InputBindings::KeyChord *chord)
{
	return chord->activator.Enabled() + chord->modifier1.Enabled() + chord->modifier2.Enabled();
}

static float applyDeadzoneAndCurve(const JoystickInfo::Axis &axis, float value)
{
	float absVal = std::fabs(value);
	float sign = value < 0.0 ? -1.0 : 1.0;
	if (absVal < axis.deadzone) return 0.0f;
	// renormalize value to 0..1 after deadzone
	absVal = (absVal - axis.deadzone) * (1.0 / (1.0 - axis.deadzone));
	return AnimationCurves::SmoothEasing(absVal, axis.curve) * sign;
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
			mouseButton[event.button.button] = 4;
			onMouseButtonUp.emit(event.button.button,
				event.button.x, event.button.y);
		}
		break;
	case SDL_MOUSEWHEEL:
		if (event.wheel.y != 0) {
			mouseWheel = event.wheel.y;
			onMouseWheel.emit(event.wheel.y > 0); // true = up
		}
		break;
	case SDL_MOUSEMOTION:
		mouseMotion[0] += event.motion.xrel;
		mouseMotion[1] += event.motion.yrel;
		break;
	case SDL_JOYAXISMOTION: {
		if (!GetJoysticks()[Input::JoystickFromID(event.jaxis.which)].joystick)
			break;
		auto &axis = GetJoysticks()[Input::JoystickFromID(event.jaxis.which)].axes[event.jaxis.axis];
		float value = (event.jaxis.value == -32768 ? -1.f : event.jaxis.value / 32767.f);

		if (axis.zeroToOne)
			axis.value = applyDeadzoneAndCurve(axis, std::abs((value + 1.0f) * 0.5f));
		else
			// by convention, positive values are mapped to left/down, negative values right/up; so we flip them here
			axis.value = applyDeadzoneAndCurve(axis, -value); // flip direction of physical axes from -left +right
	} break;
	case SDL_JOYBUTTONUP:
	case SDL_JOYBUTTONDOWN:
		if (!GetJoysticks()[Input::JoystickFromID(event.jaxis.which)].joystick)
			break;
		GetJoysticks()[Input::JoystickFromID(event.jbutton.which)].buttons[event.jbutton.button] = event.jbutton.state != 0;
		break;
	case SDL_JOYHATMOTION:
		if (!GetJoysticks()[Input::JoystickFromID(event.jaxis.which)].joystick)
			break;
		GetJoysticks()[Input::JoystickFromID(event.jhat.which)].hats[event.jhat.hat] = event.jhat.value;
		break;
	case SDL_JOYDEVICEADDED: {
		int device_id = event.jdevice.which;
		SDL_Joystick *newJoystick = SDL_JoystickOpen(device_id);
		if (!newJoystick) {
			Warning("SDL_JoystickOpen(%i): %s\n", device_id, SDL_GetError());
			break;
		}
		int id = JoystickFromGUID(SDL_JoystickGetGUID(newJoystick));
		if (id >= 0) {
			// this joystick is in the list, but not connected
			JoystickInfo &state = GetJoysticks()[id];
			state.joystick = newJoystick;
			state.buttons.resize(SDL_JoystickNumButtons(state.joystick));
			state.hats.resize(SDL_JoystickNumHats(state.joystick));
			int instance_id = SDL_JoystickInstanceID(state.joystick);
			AddJoystickID(instance_id, id);
		} else {
			// brand new joystick, never plugged in
			JoystickInfo state = loadJoystick(newJoystick, Pi::config);
			int instance_id = SDL_JoystickInstanceID(state.joystick);
			AddJoystickID(instance_id, m_joysticks.size());
			m_joysticks.push_back(state);
		}
		break;
	}
	case SDL_JOYDEVICEREMOVED: {
		int id = JoystickFromID(event.jdevice.which);
		GetJoysticks()[id].joystick = nullptr;
	} break;
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
			bool nowActive = action->binding.IsActive() || action->binding2.IsActive();
			action->m_active = nowActive;

			// if at least one of the bindings was pressed this frame and the action was not
			// previously active, call the pressed event
			if (queued & 1 && !wasActive)
				action->onPressed.emit();

			// if at least one of the bindings was released this frame but are not pressed currently,
			// call the released event
			if (queued & 2 && !nowActive)
				action->onReleased.emit();

			// clear queued events
			action->binding.m_queuedEvents = action->binding2.m_queuedEvents = 0;
		}
	}

	for (auto *axis : m_activeAxes) {
		float value = GetAxisState(axis->axis);

		value += axis->positive.IsActive();
		value -= axis->negative.IsActive();
		value += axis->m_manualValue;

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

	SDL_SetWindowGrab(m_window, SDL_bool(grabbed));
	SDL_SetRelativeMouseMode(SDL_bool(grabbed));
	m_capturingMouse = grabbed;
}

void Manager::ClearMouse()
{
	mouseButton.fill(0);
	mouseMotion.fill(0);
}

float Manager::GetMoveSpeedShiftModifier()
{
	// Suggestion: make x1000 speed on pressing both keys?
	if (KeyState(SDLK_LSHIFT)) return 100.f;
	if (KeyState(SDLK_RSHIFT)) return 10.f;
	return 1;
}
