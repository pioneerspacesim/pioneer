// Copyright © 2008-2024 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#ifndef INPUT_H
#define INPUT_H

#include "InputBindings.h"

#include "SDL_joystick.h"

#include <array>
#include <vector>
#include <map>
#include <string>

class IniConfig;

// Macro to simplify registering input bindings in the codebase
// TODO: evaluate if registering key bindings via lua / json file works better
#define REGISTER_INPUT_BINDING(name)                                   \
	namespace name##Input                                              \
	{                                                                  \
		void Register(Input::Manager *input);                          \
		bool name##Registered = Input::AddBindingRegistrar(&Register); \
	}                                                                  \
	void name##Input::Register(Input::Manager *input)

namespace Input {
	class Manager;

	// The Page->Group->Binding system serves as a thin veneer for the UI to make
	// sane reasonings about how to structure the Options dialog.
	struct BindingGroup {
		enum EntryType : uint8_t {
			ENTRY_ACTION,
			ENTRY_AXIS
		};

		std::map<std::string, EntryType> bindings;
	};

	struct BindingPage {
		BindingGroup *GetBindingGroup(std::string id) { return &groups[id]; }

		std::map<std::string, BindingGroup> groups;
	};

	struct InputFrame {
		using Axis = InputBindings::Axis;
		using Action = InputBindings::Action;

		InputFrame(Input::Manager *man, bool modal = false) :
			manager(man),
			modal(modal)
		{}

		std::vector<Action *> actions;
		std::vector<Axis *> axes;

		// Must set this to a valid Input::Manager instance before using AddAction / AddAxis
		Manager *manager = nullptr;
		bool active = false;
		bool modal = false;

		// Call this at startup to register all the bindings associated with the frame.
		virtual void RegisterBindings(){};

		// Called when the frame is added to the stack.
		sigc::signal<void, InputFrame *> onFrameAdded;

		// Called when the frame is removed from the stack.
		sigc::signal<void, InputFrame *> onFrameRemoved;

		Action *AddAction(std::string id);
		Axis *AddAxis(std::string id);
	};

	struct JoystickInfo {
		struct Axis {
			float value = 0.0;
			float deadzone = 0.1;
			float curve = 1.0;
			bool zeroToOne = false;
		};

		SDL_Joystick *joystick;
		SDL_JoystickGUID guid;
		std::string name;

		std::vector<bool> buttons;
		std::vector<int> hats;

		std::vector<Axis> axes;
	};

	void InitJoysticks(IniConfig *config);
	std::vector<JoystickInfo> &GetJoysticks();

	// User display name for the joystick from the API/OS.
	std::string JoystickName(int joystick);
	// fetch the GUID for the named joystick
	SDL_JoystickGUID JoystickGUID(int joystick);
	std::string JoystickGUIDString(int joystick);
	// update the joystick's saved configuration
	void SaveJoystickConfig(uint32_t joystick, IniConfig *config);

	// reverse map a JoystickGUID to the actual internal ID.
	int JoystickFromGUIDString(const std::string &guid);
	int JoystickFromGUIDString(const char *guid);
	int JoystickFromGUID(SDL_JoystickGUID guid);

	int JoystickFromID(SDL_JoystickID id);
	// map SDL joystick instance ID to internal ID
	void AddJoystickID(SDL_JoystickID sdl_id, uint32_t internal_id);

	// An adapter to decouple input frame creation from input binding registration.
	// The functions registered via AddBindingRegistrar should be thread-safe and
	// should not depend on anything but the manager object being passed in.
	// The registrars are guaranteed to be called after static initialization has finished.
	std::vector<sigc::slot<void, Input::Manager *>> &GetBindingRegistration();
	bool AddBindingRegistrar(sigc::slot<void, Input::Manager *> &&fn);
} // namespace Input

class Input::Manager {
public:
	Manager(IniConfig *config, SDL_Window *window);
	void InitGame();

	// Call this at the start of a frame, before passing SDL events in
	void NewFrame();

	// Call once per SDL event, handles updating all internal state
	void HandleSDLEvent(SDL_Event &ev);

	// Call immediately after processing events, dispatches events to Action / Axis bindings.
	void DispatchEvents();

	// When enable is false, this prevents the input system from writing to the config file.
	void EnableConfigSaving(bool enable) { m_enableConfigSaving = enable; }

	BindingPage *GetBindingPage(std::string id) { return &bindingPages[id]; }
	std::map<std::string, BindingPage> GetBindingPages() { return bindingPages; }

	// Pushes an InputFrame onto the input stack.
	bool AddInputFrame(InputFrame *frame);

	// Get a read-only list of input frames.
	const std::vector<InputFrame *> &GetInputFrames() { return m_inputFrames; }

	// Check if a specific input frame is currently on the stack.
	bool HasInputFrame(InputFrame *frame);

	// Remove an arbitrary input frame from the input stack.
	void RemoveInputFrame(InputFrame *frame);

	// Inform the input system that a binding or frame was changed this frame.
	void MarkBindingsDirty() { m_frameListChanged = true; }

	// Creates a new action binding, copying the provided binding.
	// The returned binding pointer points to the actual binding.
	InputBindings::Action *AddActionBinding(std::string id, BindingGroup *group, InputBindings::Action &&binding);
	InputBindings::Action *GetActionBinding(std::string id);

	// Creates a new axis binding, copying the provided binding.
	// The returned binding pointer points to the actual binding.
	InputBindings::Axis *AddAxisBinding(std::string id, BindingGroup *group, InputBindings::Axis &&binding);
	InputBindings::Axis *GetAxisBinding(std::string id);

	// Call EnableBindings() to temporarily disable handling input bindings while
	// you're recording a new input binding or are in a modal window.
	void EnableBindings(bool enabled) { m_enableBindings = enabled; }

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
	void SetJoystickEnabled(bool state);

	bool IsMouseYInvert() { return mouseYInvert; }
	void SetMouseYInvert(bool state);

	bool IsMouseButtonPressed(int button) { return mouseButton[button] == 1; }
	bool IsMouseButtonReleased(int button) { return mouseButton[button] == 4; }

	bool MouseButtonState(int button) { return mouseButton[button] & 3; }
	void SetMouseButtonState(int button, bool state) { mouseButton[button] = state; }

	void GetMousePosition(int position[2]);

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
	void ClearMouse();

	// Get the default speed modifier to apply to movement (scrolling, zooming...), depending on the "shift" keys.
	// This is a default value only, centralized here to promote uniform user expericience.
	float GetMoveSpeedShiftModifier();

	sigc::signal<void, SDL_Keysym *> onKeyPress;
	sigc::signal<void, SDL_Keysym *> onKeyRelease;
	sigc::signal<void, int, int, int> onMouseButtonUp;
	sigc::signal<void, int, int, int> onMouseButtonDown;
	sigc::signal<void, bool> onMouseWheel;

private:
	void RebuildInputFrames();
	bool GetModifierState(InputBindings::KeyChord *key);
	bool GetBindingState(InputBindings::KeyBinding &key);
	float GetAxisState(InputBindings::JoyAxis &axis);

	SDL_Window *m_window;
	IniConfig *m_config;
	bool m_enableConfigSaving;

	std::map<SDL_Keycode, uint8_t> keyState;
	int keyModState;
	std::array<char, 6> mouseButton;
	std::array<int, 2> mouseMotion;
	int mouseWheel;
	bool m_capturingMouse;

	bool joystickEnabled;
	bool mouseYInvert;

	std::map<std::string, BindingPage> bindingPages;
	std::map<std::string, InputBindings::Action> actionBindings;
	std::map<std::string, InputBindings::Axis> axisBindings;
	bool m_enableBindings;

	std::vector<InputFrame *> m_inputFrames;
	bool m_frameListChanged;

	std::vector<InputBindings::Action *> m_activeActions;
	std::vector<InputBindings::Axis *> m_activeAxes;

	std::map<InputBindings::KeyBinding, bool> m_modifiers;
	std::vector<InputBindings::KeyChord *> m_chords;
};

#endif
