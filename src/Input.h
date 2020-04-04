// Copyright © 2008-2020 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#ifndef INPUT_H
#define INPUT_H

#include "KeyBindings.h"
#include "utils.h"

#include <algorithm>
#include <array>

class IniConfig;

class Input {
public:
	Input(IniConfig *config);
	void InitGame();
	void HandleSDLEvent(SDL_Event &ev);
	void NewFrame();

	// The Page->Group->Binding system serves as a thin veneer for the UI to make
	// sane reasonings about how to structure the Options dialog.
	struct BindingGroup {
		enum EntryType {
			ENTRY_ACTION,
			ENTRY_AXIS
		};

		std::map<std::string, EntryType> bindings;
	};

	struct BindingPage {
		BindingGroup *GetBindingGroup(std::string id) { return &groups[id]; }

		std::map<std::string, BindingGroup> groups;
	};

	BindingPage *GetBindingPage(std::string id) { return &bindingPages[id]; }
	std::map<std::string, BindingPage> GetBindingPages() { return bindingPages; }

	struct InputFrame {
		std::vector<KeyBindings::ActionBinding *> actions;
		std::vector<KeyBindings::AxisBinding *> axes;

		bool active;

		// Call this at startup to register all the bindings associated with the frame.
		virtual void RegisterBindings(){};

		// Called when the frame is added to the stack.
		virtual void onFrameAdded(){};

		// Called when the frame is removed from the stack.
		virtual void onFrameRemoved(){};

		// Check the event against all the inputs in this frame.
		InputResponse ProcessSDLEvent(SDL_Event &event);
	};

	// Pushes an InputFrame onto the input stack.
	bool PushInputFrame(InputFrame *frame);

	// Pops the most-recently pushed InputFrame from the stack.
	InputFrame *PopInputFrame();

	// Get a read-only list of input frames.
	const std::vector<InputFrame *> &GetInputFrames() { return inputFrames; }

	// Check if a specific input frame is currently on the stack.
	bool HasInputFrame(InputFrame *frame)
	{
		return std::count(inputFrames.begin(), inputFrames.end(), frame) > 0;
	}

	// Remove an arbitrary input frame from the input stack.
	void RemoveInputFrame(InputFrame *frame);

	// Creates a new action binding, copying the provided binding.
	// The returned binding pointer points to the actual binding.
	KeyBindings::ActionBinding *AddActionBinding(std::string id, BindingGroup *group, KeyBindings::ActionBinding binding);
	KeyBindings::ActionBinding *GetActionBinding(std::string id)
	{
		return actionBindings.count(id) ? &actionBindings[id] : nullptr;
	}

	// Creates a new axis binding, copying the provided binding.
	// The returned binding pointer points to the actual binding.
	KeyBindings::AxisBinding *AddAxisBinding(std::string id, BindingGroup *group, KeyBindings::AxisBinding binding);
	KeyBindings::AxisBinding *GetAxisBinding(std::string id)
	{
		return axisBindings.count(id) ? &axisBindings[id] : nullptr;
	}

	bool KeyState(SDL_Keycode k) { return IsKeyDown(k); }

	// returns true if key K is currently pressed
	bool IsKeyDown(SDL_Keycode k) { return keyState[k] & 0x3; }

	// returns true if key K was pressed this frame
	bool IsKeyPressed(SDL_Keycode k) { return keyState[k] == 1; }

	// returns true if key K was released this frame
	bool IsKeyReleased(SDL_Keycode k) { return keyState[k] == 4; }

	int KeyModState() { return keyModState; }

	int JoystickButtonState(int joystick, int button);
	int JoystickHatState(int joystick, int hat);
	float JoystickAxisState(int joystick, int axis);

	bool IsJoystickEnabled() { return joystickEnabled; }
	void SetJoystickEnabled(bool state) { joystickEnabled = state; }

	struct JoystickState {
		SDL_Joystick *joystick;
		SDL_JoystickGUID guid;
		std::vector<bool> buttons;
		std::vector<int> hats;
		std::vector<float> axes;
	};
	std::map<SDL_JoystickID, JoystickState> GetJoysticksState() { return joysticks; }

	// User display name for the joystick from the API/OS.
	std::string JoystickName(int joystick);
	// fetch the GUID for the named joystick
	SDL_JoystickGUID JoystickGUID(int joystick);
	std::string JoystickGUIDString(int joystick);

	// reverse map a JoystickGUID to the actual internal ID.
	int JoystickFromGUIDString(const std::string &guid);
	int JoystickFromGUIDString(const char *guid);
	int JoystickFromGUID(SDL_JoystickGUID guid);

	void SetMouseYInvert(bool state) { mouseYInvert = state; }
	bool IsMouseYInvert() { return mouseYInvert; }

	int MouseButtonState(int button) { return mouseButton[button]; }
	void SetMouseButtonState(int button, bool state) { mouseButton[button] = state; }

	void GetMouseMotion(int motion[2])
	{
		std::copy_n(mouseMotion.data(), mouseMotion.size(), motion);
	}

	int GetMouseWheel() { return mouseWheel; }

	// Capturing the mouse hides the cursor, puts the mouse into relative mode,
	// and passes all mouse inputs to the input system, regardless of whether
	// ImGui is using them or not.
	bool IsCapturingMouse() const { return m_capturingMouse; }

	// Set whether the application would like to capture the mouse.
	// To avoid contention between different classes, please only call this when the state
	// has actually changed.
	void SetCapturingMouse(bool enabled);

	sigc::signal<void, SDL_Keysym *> onKeyPress;
	sigc::signal<void, SDL_Keysym *> onKeyRelease;
	sigc::signal<void, int, int, int> onMouseButtonUp;
	sigc::signal<void, int, int, int> onMouseButtonDown;
	sigc::signal<void, bool> onMouseWheel;

private:
	void InitJoysticks();

	IniConfig *m_config;

	std::map<SDL_Keycode, uint8_t> keyState;
	int keyModState;
	std::array<char, 6> mouseButton;
	std::array<int, 2> mouseMotion;
	int mouseWheel;
	bool m_capturingMouse;

	bool joystickEnabled;
	bool mouseYInvert;
	std::map<SDL_JoystickID, JoystickState> joysticks;

	std::map<std::string, BindingPage> bindingPages;
	std::map<std::string, KeyBindings::ActionBinding> actionBindings;
	std::map<std::string, KeyBindings::AxisBinding> axisBindings;

	std::vector<InputFrame *> inputFrames;
};

#endif
