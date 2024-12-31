// Copyright © 2008-2025 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#include "LuaInput.h"

#include "ConnectionTicket.h"
#include "GameConfig.h"
#include "Input.h"
#include "InputBindings.h"
#include "LuaMetaType.h"
#include "LuaObject.h"
#include "LuaUtils.h"
#include "LuaWrappable.h"
#include "Pi.h"

#include "core/StringUtils.h"

#include <SDL_keyboard.h>
#include <SDL_keycode.h>
#include <sstream>

using namespace InputBindings;

static std::vector<std::pair<const char *, int>> s_input_keys = {
	// Symbol keys

	{ "exclaim", SDLK_EXCLAIM },
	{ "quotedbl", SDLK_QUOTEDBL },
	{ "hash", SDLK_HASH },
	{ "percent", SDLK_PERCENT },
	{ "dollar", SDLK_DOLLAR },
	{ "ampersand", SDLK_AMPERSAND },
	{ "quote", SDLK_QUOTE },
	{ "leftparen", SDLK_LEFTPAREN },
	{ "rightparen", SDLK_RIGHTPAREN },
	{ "asterisk", SDLK_ASTERISK },
	{ "plus", SDLK_PLUS },
	{ "comma", SDLK_COMMA },
	{ "minus", SDLK_MINUS },
	{ "period", SDLK_PERIOD },
	{ "slash", SDLK_SLASH },

	{ "colon", SDLK_COLON },
	{ "semicolon", SDLK_SEMICOLON },
	{ "less", SDLK_LESS },
	{ "equals", SDLK_EQUALS },
	{ "greater", SDLK_GREATER },
	{ "question", SDLK_QUESTION },
	{ "at", SDLK_AT },

	{ "leftbracket", SDLK_LEFTBRACKET },
	{ "backslash", SDLK_BACKSLASH },
	{ "rightbracket", SDLK_RIGHTBRACKET },
	{ "caret", SDLK_CARET },
	{ "underscore", SDLK_UNDERSCORE },
	{ "backquote", SDLK_BACKQUOTE },

	// Misc. keys
	{ "return", SDLK_RETURN },
	{ "escape", SDLK_ESCAPE },
	{ "backspace", SDLK_BACKSPACE },
	{ "tab", SDLK_TAB },
	{ "space", SDLK_SPACE },

	{ "capslock", SDLK_CAPSLOCK },
	{ "printscreen", SDLK_PRINTSCREEN },
	{ "scrolllock", SDLK_SCROLLLOCK },
	{ "pause", SDLK_PAUSE },
	{ "insert", SDLK_INSERT },
	{ "home", SDLK_HOME },
	{ "pageup", SDLK_PAGEUP },
	{ "delete", SDLK_DELETE },
	{ "end", SDLK_END },
	{ "pagedown", SDLK_PAGEDOWN },
	{ "right", SDLK_RIGHT },
	{ "left", SDLK_LEFT },
	{ "down", SDLK_DOWN },
	{ "up", SDLK_UP },

	{ "numlockclear", SDLK_NUMLOCKCLEAR },
	{ "kp_divide", SDLK_KP_DIVIDE },
	{ "kp_multiply", SDLK_KP_MULTIPLY },
	{ "kp_minus", SDLK_KP_MINUS },
	{ "kp_plus", SDLK_KP_PLUS },
	{ "kp_enter", SDLK_KP_ENTER },
	{ "kp_1", SDLK_KP_1 },
	{ "kp_2", SDLK_KP_2 },
	{ "kp_3", SDLK_KP_3 },
	{ "kp_4", SDLK_KP_4 },
	{ "kp_5", SDLK_KP_5 },
	{ "kp_6", SDLK_KP_6 },
	{ "kp_7", SDLK_KP_7 },
	{ "kp_8", SDLK_KP_8 },
	{ "kp_9", SDLK_KP_9 },
	{ "kp_0", SDLK_KP_0 },
	{ "kp_period", SDLK_KP_PERIOD },

	{ "lctrl", SDLK_LCTRL },
	{ "lshift", SDLK_LSHIFT },
	{ "lalt", SDLK_LALT },
	{ "lgui", SDLK_LGUI },
	{ "rctrl", SDLK_RCTRL },
	{ "rshift", SDLK_RSHIFT },
	{ "ralt", SDLK_RALT },
	{ "rgui", SDLK_RGUI },

	// Function keys

	{ "f1", SDLK_F1 },
	{ "f2", SDLK_F2 },
	{ "f3", SDLK_F3 },
	{ "f4", SDLK_F4 },
	{ "f5", SDLK_F5 },
	{ "f6", SDLK_F6 },
	{ "f7", SDLK_F7 },
	{ "f8", SDLK_F8 },
	{ "f9", SDLK_F9 },
	{ "f10", SDLK_F10 },
	{ "f11", SDLK_F11 },
	{ "f12", SDLK_F12 },

	// letters

	{ "a", SDLK_a },
	{ "b", SDLK_b },
	{ "c", SDLK_c },
	{ "d", SDLK_d },
	{ "e", SDLK_e },
	{ "f", SDLK_f },
	{ "g", SDLK_g },
	{ "h", SDLK_h },
	{ "i", SDLK_i },
	{ "j", SDLK_j },
	{ "k", SDLK_k },
	{ "l", SDLK_l },
	{ "m", SDLK_m },
	{ "n", SDLK_n },
	{ "o", SDLK_o },
	{ "p", SDLK_p },
	{ "q", SDLK_q },
	{ "r", SDLK_r },
	{ "s", SDLK_s },
	{ "t", SDLK_t },
	{ "u", SDLK_u },
	{ "v", SDLK_v },
	{ "w", SDLK_w },
	{ "x", SDLK_x },
	{ "y", SDLK_y },
	{ "z", SDLK_z },

	// numbers

	{ "0", SDLK_0 },
	{ "1", SDLK_1 },
	{ "2", SDLK_2 },
	{ "3", SDLK_3 },
	{ "4", SDLK_4 },
	{ "5", SDLK_5 },
	{ "6", SDLK_6 },
	{ "7", SDLK_7 },
	{ "8", SDLK_8 },
	{ "9", SDLK_9 },
};

static LuaRef s_connections;

// Helper wrapping a ConnectionTicket to clean up a Lua callback associated with an InputAction
struct LuaInputCallback : public LuaWrappable {
	ConnectionTicket connection;
};

// Store a callback ConnectionTicket so it can be cleaned up later
static void store_callback(lua_State *l, sigc::connection &&connection, int cb_idx)
{
	LUA_DEBUG_START(l);

	std::string moduleName = pi_lua_get_caller_module(l);

	s_connections.PushCopyToStack();

	// Associate the callback with the module name
	luaL_getsubtable(l, -1, moduleName.c_str());
	lua_pushvalue(l, cb_idx);
	lua_rawseti(l, -2, luaL_len(l, -2) + 1);
	lua_pop(l, 1);

	lua_pushvalue(l, cb_idx); // s_connections, callback
	lua_rawget(l, -2);

	// Create the list of connections for this callback
	if (lua_isnil(l, -1)) {
		lua_pop(l, 1);
		lua_newtable(l);
		lua_pushvalue(l, cb_idx);
		lua_pushvalue(l, -2); // s_connections, newtab, callback, newtab(2)
		lua_rawset(l, -4);
	}

	// Stack: s_connections, connections
	LuaObject<LuaInputCallback>::PushToLua(LuaInputCallback {});
	LuaInputCallback *cb = LuaPull<LuaInputCallback *>(l, -1);

	cb->connection = std::move(connection);
	lua_rawseti(l, -2, luaL_len(l, -2) + 1);

	lua_pop(l, 2);
	LUA_DEBUG_END(l, 0);
}

// Cleanup a callback ConnectionTicket
static bool unstore_callback(lua_State *l, int cb_idx)
{
	LUA_DEBUG_START(l);

	s_connections.PushCopyToStack();
	lua_pushvalue(l, cb_idx);
	lua_rawget(l, -2);

	// No connections, don't need to do anything
	if (lua_isnil(l, -1)) {
		lua_pop(l, 2);

		return false;
	}

	// Iterate the list of connections and disconnect them
	// This will destroy the C++ lambda and the associated LuaRef
	lua_pushnil(l);
	while (lua_next(l, -2)) {
		LuaInputCallback *cb = LuaPull<LuaInputCallback *>(l, -1);
		cb->connection.m_connection.disconnect();
		lua_pop(l, 1);
	}

	lua_pushvalue(l, cb_idx);
	lua_pushnil(l);
	lua_rawset(l, -3);

	lua_pop(l, 1);
	LUA_DEBUG_END(l, 0);
	return true;
}

/*
 * Class: LuaInputAction
 *
 * Proxy object for an action binding
 *
 */

// TODO: push the actual action pointer to lua
struct LuaInputAction : public LuaWrappable {
	LuaInputAction(const std::string &_id) :
		id(_id) {}
	std::string id;

	const char *getType() const { return "Action"; }
	KeyChord getBinding() const { return getAction()->binding; }
	KeyChord getBinding2() const { return getAction()->binding2; }

	void setBinding(KeyChord &bind)
	{
		auto *action = getAction();
		if (action->binding != bind) {
			action->binding = bind;
			Pi::input->MarkBindingsDirty();
		}
	}

	void setBinding2(KeyChord &bind)
	{
		auto *action = getAction();
		if (action->binding2 != bind) {
			action->binding2 = bind;
			Pi::input->MarkBindingsDirty();
		}
	}

	/*
	 * Function: SetPressed
	 *
	 * Activate the corresponding action
	 *
	 * Example:
	 *
	 * > action = Input.GetActionBinding("BindToggleSpeedLimiter")
	 * > if ui.isMouseClicked(0) then
	 * >     action:SetPressed()
	 * > elseif ui.isMouseReleased(0) then
	 * >     action:SetReleased()
	 * > end
	 *
	 * Returns:
	 *
	 *   nothing
	 *
	 */
	void setPressed()
	{
		auto *action = getAction();
		action->binding.m_queuedEvents |= 1;
		action->binding.m_active = true;
	}

	/*
	 * Function: SetReleased
	 *
	 * Deactivate the corresponding action
	 *
	 * Example:
	 *
	 * > action = Input.GetActionBinding("BindToggleSpeedLimiter")
	 * > if ui.isMouseClicked(0) then
	 * >     action:SetPressed()
	 * > elseif ui.isMouseReleased(0) then
	 * >     action:SetReleased()
	 * > end
	 *
	 * Returns:
	 *
	 *   nothing
	 *
	 */
	void setReleased()
	{
		auto *action = getAction();
		action->binding.m_queuedEvents |= 2;
		action->binding.m_active = false;
	}

	/*
	 * Function: IsActive
	 *
	 * Check if the corresponding action is currently active
	 *
	 * Example:
	 *
	 * > action = Input.GetActionBinding("BindToggleSpeedLimiter")
	 * > active = action:IsActive()
	 *
	 * Returns:
	 *
	 *   boolean
	 *
	 */
	bool isActive()
	{
		auto *action = getAction();
		return action->binding.IsActive() || action->binding2.IsActive();
	}

	/*
	 * Function: IsJustActive
	 *
	 * Check if the corresponding action is activated in the current frame
	 *
	 * Example:
	 *
	 * > action = Input.GetActionBinding("BindToggleSpeedLimiter")
	 * > activated = action:IsJustActive()
	 *
	 * Returns:
	 *
	 *   boolean
	 *
	 */
	bool isJustActive()
	{
		auto *action = getAction();
		return (action->binding.m_queuedEvents & 1) || (action->binding2.m_queuedEvents & 1);
	}

	Action *getAction() const { return Pi::input->GetActionBinding(id); }

	/**
	 * Function: OnPressed()
	 *
	 * Register a Lua callback function to be called when this action is
	 * pressed while in an active InputFrame.
	 */
	static int AddOnPressedCallback(lua_State *l, LuaInputAction *action)
	{
		LuaRef cbRef = LuaRef(l, 2);
		auto connection = action->getAction()->onPressed.connect([cbRef]() {
			cbRef.PushCopyToStack();
			pi_lua_protected_call(cbRef.GetLua(), 0, 0);
		});

		store_callback(l, connection, 2);
		return 0;
	}

	/**
	 * Function: OnReleased()
	 *
	 * Register a Lua callback function to be called when this action is
	 * released while in an active InputFrame.
	 */
	static int AddOnReleasedCallback(lua_State *l, LuaInputAction *action)
	{
		LuaRef cbRef = LuaRef(l, 2);
		auto connection = action->getAction()->onReleased.connect([cbRef]() {
			cbRef.PushCopyToStack();
			pi_lua_protected_call(cbRef.GetLua(), 0, 0);
		});

		store_callback(l, connection, 2);
		return 0;
	}
};

/*
 * Class: LuaInputAxis
 *
 * Proxy object for an axis binding
 *
 */

// TODO: push the actual axis pointer to lua
struct LuaInputAxis : public LuaWrappable {
	LuaInputAxis(const std::string &_id) :
		id(_id) {}
	std::string id;

	const char *getType() const { return "Axis"; }
	JoyAxis getAxisBinding() const { return getAxis()->axis; }
	KeyChord getNegative() const { return getAxis()->negative; }
	KeyChord getPositive() const { return getAxis()->positive; }

	void setAxisBinding(JoyAxis &axis)
	{
		getAxis()->axis = axis;
		Pi::input->MarkBindingsDirty();
	}

	void setNegative(KeyChord &chord)
	{
		auto axis = getAxis();
		if (axis->negative != chord) {
			axis->negative = chord;
			Pi::input->MarkBindingsDirty();
		}
	}

	void setPositive(KeyChord &chord)
	{
		auto axis = getAxis();
		if (axis->positive != chord) {
			axis->positive = chord;
			Pi::input->MarkBindingsDirty();
		}
	}

	/*
	 * Function: SetValue
	 *
	 * Set the corresponding axis value
	 *
	 * Example:
	 *
	 * > axis = Input.GetAxisBinding('BindAxisYaw')
	 * > value = ui.sliderFloat("", value, -1, 1, "%.3f")
	 * > axis:SetValue(value)
	 *
	 * Parameters:
	 *
	 * value: real number, -1.0 .. 1.0
	 *
	 * Returns:
	 *
	 *   nothing
	 *
	 */
	void setValue(double value)
	{
		getAxis()->SetValue(value);
	}

	/*
	 * Function: GetValue
	 *
	 * Get the corresponding axis value
	 *
	 * Example:
	 *
	 * > axis = Input.GetAxisBinding('BindAxisYaw')
	 * > slider_value = axis:GetValue()
	 * > ui.sliderFloat("", slider_value, -1, 1, "%.3f")
	 *
	 * Returns:
	 *
	 *   real number, -1.0 .. 1.0
	 *
	 */
	double getValue()
	{
		return getAxis()->GetValue();
	}

	Axis *getAxis() const { return Pi::input->GetAxisBinding(id); }

	/**
	 * Function: OnValueChanged()
	 *
	 * Register a Lua callback function to be called when this axis' value is
	 * changed while part of an active InputFrame.
	 */
	static int AddValueCallback(lua_State *l, LuaInputAxis *axis)
	{
		LuaRef cbRef = LuaRef(l, 2);
		auto connection = axis->getAxis()->onAxisValue.connect([cbRef](float value) {
			cbRef.PushCopyToStack();
			lua_pushnumber(cbRef.GetLua(), value);
			pi_lua_protected_call(cbRef.GetLua(), 1, 0);
		});

		store_callback(l, connection, 2);
		return 0;
	}
};

/**
 * Class: LuaJoystickInfo
 *
 * Represents information about a specific joystick instance connected to the system.
 */
struct LuaJoystickInfo : public LuaWrappable {
	LuaJoystickInfo(int joystickIndex) :
		m_id(joystickIndex)
	{}

	const char *getName() const { return getInfo()->name.c_str(); }

	uint32_t numButtons() const { return getInfo()->buttons.size(); }
	uint32_t numHats() const { return getInfo()->hats.size(); }
	uint32_t numAxes() const { return getInfo()->axes.size(); }

	bool getButtonState(uint32_t button) const { return button < getInfo()->buttons.size() ? getInfo()->buttons[button] : false; }
	bool getHatState(uint32_t hat) const { return hat < getInfo()->hats.size() ? getInfo()->hats[hat] : false; }
	float getAxisValue(uint32_t axis) const { return axis < getInfo()->axes.size() ? getInfo()->axes[axis].value : false; }

	float getAxisDeadzone(uint32_t axis) const { return axis < getInfo()->axes.size() ? getInfo()->axes[axis].deadzone : 0.0f; }

	bool setAxisDeadzone(uint32_t axis, float dz) {
		auto *js = getInfo();
		if (axis >= js->axes.size())
			return false;

		js->axes[axis].deadzone = dz;
		return true;
	}

	float getAxisCurve(uint32_t axis) const { return axis < getInfo()->axes.size() ? getInfo()->axes[axis].curve : 0.0f; }

	bool setAxisCurve(uint32_t axis, float curve) {
		auto *js = getInfo();
		if (axis >= js->axes.size())
			return false;

		js->axes[axis].curve = curve;
		return true;
	}

	bool getAxisZeroToOne(uint32_t axis) const { return axis < getInfo()->axes.size() ? getInfo()->axes[axis].zeroToOne : false; }

	bool setAxisZeroToOne(uint32_t axis, bool enabled) {
		auto *js = getInfo();
		if (axis >= js->axes.size())
			return false;

		js->axes[axis].zeroToOne = enabled;
		return true;
	}

private:
	Input::JoystickInfo *getInfo() const { return &Input::GetJoysticks()[m_id]; }
	int m_id;
};

/**
 * Class: LuaInputFrame
 *
 * Collects a group of input bindings and controls pushing them to the active
 * input stack.
 */
struct LuaInputFrame : public LuaWrappable {
	LuaInputFrame(Input::Manager *manager, bool modal, std::string_view id) :
		m_inputFrame(manager, modal, id)
	{
	}

	~LuaInputFrame()
	{
		if (m_inputFrame.manager->HasInputFrame(&m_inputFrame)) {
			Log::Warning("LuaInput: InputFrame {} being removed from input stack in __gc callback.", m_inputFrame.id);
			m_inputFrame.manager->RemoveInputFrame(&m_inputFrame);
		}
	}

	const std::string &getId() const {
		return m_inputFrame.id;
	}

	bool addToStack() {
		return m_inputFrame.manager->AddInputFrame(&m_inputFrame);
	}

	void removeFromStack() {
		return m_inputFrame.manager->RemoveInputFrame(&m_inputFrame);
	}

	void addAction(LuaInputAction *action) {
		m_inputFrame.AddAction(action->id);
	}

	void addAxis(LuaInputAxis *axis) {
		m_inputFrame.AddAxis(axis->id);
	}

private:
	Input::InputFrame m_inputFrame;
};

#define GENERIC_COPY_OBJ_DEF(Typename)                                       \
	template <>                                                              \
	const char *LuaObject<Typename>::s_type = #Typename;                     \
	inline void pi_lua_generic_push(lua_State *l, const Typename &value)     \
	{                                                                        \
		assert(l == Lua::manager->GetLuaState());                            \
		LuaObject<Typename>::PushToLua(value);                               \
	}

GENERIC_COPY_OBJ_DEF(LuaInputCallback)
GENERIC_COPY_OBJ_DEF(LuaInputAction)
GENERIC_COPY_OBJ_DEF(LuaInputAxis)
GENERIC_COPY_OBJ_DEF(LuaJoystickInfo)
GENERIC_COPY_OBJ_DEF(LuaInputFrame)

/*
 * Interface: Input
 *
 * A global table that exposes the engine's Input interface. Key bindings and
 * state can be queried here.
 *
 */

static void setup_binding_table(lua_State *l, const char *id, const char *type)
{
	lua_newtable(l);

	lua_pushstring(l, id); // set the "id" field to the id of the page
	lua_setfield(l, -2, "id");

	lua_pushstring(l, type); // set the "type" field to "Page"
	lua_setfield(l, -2, "type");
}

/*
 * Function: GetBindingPages
 *
 * Get a table listing all the current key and axis bindings.
 *
 * > bindings = Input.GetBindingPages()
 *
 * Returns:
 *
 *   bindings - A table containing all the key and axis bindings.
 *
 * The bindings table has the following structure (in Lua syntax):
 *
 * > bindings = {
 * >	{ -- a page
 * >		id = 'CONTROLS', -- the translation key of the page's label
 * >		type = 'page',
 * >		{ -- a group
 * >			id = 'Miscellaneous', -- the translation key of the name of the group
 * >			type = 'group',
 * >			{ -- a LuaInputAction binding
 * >				type = 'Action', -- the type of binding; can be 'Action' or 'Axis'
 * >				id = 'BindToggleLuaConsole', -- the internal ID of the binding; used as a translation key and passed to Input.SetKeyBinding
 * >				binding = { enabled = true, activator = { key = 93 }, modifier1 = nil, modifier2 = nil },
 * >				binding2 = { enabled = true, activator = { key = 104 }, modifier1 = { joystick = 2, button = 7 }, modifier2 = nil }
 * >			},
 * >			{ -- a LuaInputAxis binding
 * >				type = 'Axis',
 * >				id = 'BindAxisPitch',
 * >				axis = { joystick = 2, axis = 1 },
 * >				negative = { enabled = false },
 * >				positive = { enabled = false }
 * >			}
 * >			-- ... more bindings
 * >		},
 * >		-- ... more groups
 * >	},
 * >	-- ... more pages
 * > }
 *
 * Availability:
 *
 *   September 2018
 *
 * Status:
 *
 *   permanent
 */
static int l_input_get_binding_pages(lua_State *l)
{
	LUA_DEBUG_START(l);

	lua_newtable(l); // [-1] bindings
	using namespace InputBindings;

	int page_idx = 1;
	for (auto page : Pi::input->GetBindingPages()) {
		lua_pushunsigned(l, page_idx++);
		setup_binding_table(l, page.first.c_str(), "page");

		int group_idx = 1;
		for (auto group : page.second.groups) {
			lua_pushunsigned(l, group_idx++);
			setup_binding_table(l, group.first.c_str(), "group");

			int binding_idx = 1;
			for (auto type : group.second.bindings) {
				lua_pushunsigned(l, binding_idx++);
				if (type.second == Input::BindingGroup::EntryType::ENTRY_ACTION)
					LuaPush(l, LuaInputAction{ type.first });
				else
					LuaPush(l, LuaInputAxis{ type.first });

				// [-3] group, [-2] idx, [-1] binding
				lua_settable(l, -3);
			}

			// [-3] page, [-2] idx, [-1] group
			lua_settable(l, -3);
		}

		// [-3] bindings, [-2] idx, [-1] page
		lua_settable(l, -3);
	}

	LUA_DEBUG_END(l, 1);
	return 1;
}

static int l_input_enable_bindings(lua_State *l)
{
	bool enable = lua_gettop(l) > 0 ? lua_toboolean(l, 1) : true;
	Pi::input->EnableBindings(enable);
	return 0;
}

static int l_input_save_binding(lua_State *l)
{
	LuaInputAction *action = LuaObject<LuaInputAction>::GetFromLua(1);
	LuaInputAxis *axis = LuaObject<LuaInputAxis>::GetFromLua(1);

	std::ostringstream buffer;
	if (action) {
		buffer << *action->getAction();
		Pi::config->SetString(action->id, buffer.str());
		Pi::config->Save();
	} else if (axis) {
		buffer << *axis->getAxis();
		Pi::config->SetString(axis->id, buffer.str());
		Pi::config->Save();
	}

	return 0;
}

static int l_input_get_key_name(lua_State *l)
{
	auto name = LuaPull<int>(l, 1);
	lua_pushstring(l, SDL_GetKeyName(name));
	return 1;
}

static int l_input_get_joystick_name(lua_State *l)
{
	auto joystick = LuaPull<int>(l, 1);
	lua_pushstring(l, Input::JoystickName(joystick).c_str());
	return 1;
}

static int l_input_get_joystick(lua_State *l)
{
	uint32_t joystick = LuaPull<uint32_t>(l, 1);
	if (joystick < Input::GetJoysticks().size())
		LuaPush(l, LuaJoystickInfo(joystick));
	else
		lua_pushnil(l);

	return 1;
}

static int l_input_save_joystick_config(lua_State *l)
{
	uint32_t joystick = LuaPull<uint32_t>(l, 1);
	if (joystick < Input::GetJoysticks().size()) {
		Input::SaveJoystickConfig(joystick, Pi::config);
		Pi::config->Save();
	}

	return 0;
}

/*
 * Function: IsJoystickConnected
 *
 * > connected = Input.IsJoystickConnected(id)
 *
 * Parameters:
 *
 *   id - joystick internal ID, non-negative integer number less than Input.GetJoystickCount()
 *
 * Returns:
 *
 *   boolean
 *
 */
static int l_input_is_joystick_connected(lua_State *l)
{
	auto joystick = LuaPull<int>(l, 1);
	LuaPush(l, Input::GetJoysticks()[joystick].joystick != nullptr);
	return 1;
}

/*
 * Function: GetJoystickCount
 *
 * Return the number of known joysticks in the game
 *
 * Example:
 *
 * > joysticks = Input.GetJoystickCount()
 *
 * Returns:
 *
 *   number
 *
 */
static int l_input_get_joystick_count(lua_State *l)
{
	LuaPush<int>(l, Input::GetJoysticks().size());
	return 1;
}

static int l_input_get_action_binding(lua_State *l)
{
	std::string id = luaL_checkstring(l, 1);
	auto *binding = Pi::input->GetActionBinding(id);
	if (!binding) {
		lua_pushnil(l);
		return 1;
	}

	LuaPush(l, LuaInputAction{ id });
	return 1;
}

static int l_input_get_axis_binding(lua_State *l)
{
	std::string id = luaL_checkstring(l, 1);
	auto *binding = Pi::input->GetAxisBinding(id);
	if (!binding) {
		lua_pushnil(l);
		return 1;
	}

	LuaPush(l, LuaInputAxis{ id });
	return 1;
}

static int l_input_get_mouse_y_inverted(lua_State *l)
{
	lua_pushboolean(l, Pi::input->IsMouseYInvert());
	return 1;
}

static int l_input_set_mouse_y_inverted(lua_State *l)
{
	if (lua_isnone(l, 1))
		return luaL_error(l, "SetMouseYInverted takes one boolean argument");
	const bool inverted = lua_toboolean(l, 1);
	Pi::config->SetInt("InvertMouseY", (inverted ? 1 : 0));
	Pi::config->Save();
	Pi::input->SetMouseYInvert(inverted);
	return 0;
}

static int l_input_get_mouse_captured(lua_State *l)
{
	LuaPush<bool>(l, Pi::input->IsCapturingMouse());
	return 1;
}

static int l_input_get_joystick_enabled(lua_State *l)
{
	lua_pushboolean(l, Pi::input->IsJoystickEnabled());
	return 1;
}

static int l_input_set_joystick_enabled(lua_State *l)
{
	if (lua_isnone(l, 1))
		return luaL_error(l, "SetJoystickEnabled takes one boolean argument");
	const bool enabled = lua_toboolean(l, 1);
	Pi::config->SetInt("EnableJoystick", (enabled ? 1 : 0));
	Pi::config->Save();
	Pi::input->SetJoystickEnabled(enabled);
	return 0;
}

static int l_input_create_input_frame(lua_State *l)
{
	std::string_view id = LuaPull<std::string_view>(l, 1);
	bool modal = LuaPull<bool>(l, 2, false);

	LuaObject<LuaInputFrame>::PushToLua(LuaInputFrame(Pi::input, modal, id));
	return 1;
}

static int l_input_register_action_binding(lua_State *l)
{
	std::string id = LuaPull<std::string>(l, 1);
	std::string_view groupId = LuaPull<std::string_view>(l, 2);

	if (Pi::input->HasActionBinding(id)) {
		Log::Info("LuaInput: action binding {} already exists, not overwriting.", id);
		LuaPush<LuaInputAction>(l, id);
		return 1;
	}

	Input::BindingPage *page = nullptr;
	Input::BindingGroup *group = nullptr;

	for (std::string_view id : SplitString(groupId, ".")) {
		if (page == nullptr) {
			page = Pi::input->GetBindingPage(std::string(id));
		} else if (group == nullptr) {
			group = page->GetBindingGroup(std::string(id));
			break;
		}
	}

	auto primary = LuaPull<InputBindings::KeyChord>(l, 3, {});
	auto secondary = LuaPull<InputBindings::KeyChord>(l, 4, {});

	Pi::input->AddActionBinding(id, group, InputBindings::Action(primary, secondary));

	LuaPush<LuaInputAction>(l, id);
	return 1;
}

static int l_input_register_axis_binding(lua_State *l)
{
	std::string id = LuaPull<std::string>(l, 1);
	std::string_view groupId = LuaPull<std::string_view>(l, 2);

	if (Pi::input->HasAxisBinding(id)) {
		Log::Info("LuaInput: axis binding {} already exists, not overwriting.", id);
		LuaPush<LuaInputAxis>(l, id);
		return 1;
	}

	Input::BindingPage *page = nullptr;
	Input::BindingGroup *group = nullptr;

	for (std::string_view id : SplitString(groupId, ".")) {
		if (page == nullptr) {
			page = Pi::input->GetBindingPage(std::string(id));
		} else if (group == nullptr) {
			group = page->GetBindingGroup(std::string(id));
			break;
		}
	}

	auto positive = LuaPull<InputBindings::KeyChord>(l, 3, {});
	auto negative = LuaPull<InputBindings::KeyChord>(l, 4, {});
	auto axis = LuaPull<InputBindings::JoyAxis>(l, 5, {});

	Pi::input->AddAxisBinding(id, group, InputBindings::Axis(axis, positive, negative));

	LuaPush<LuaInputAxis>(l, id);
	return 1;
}

static int l_input_disconnect_callback(lua_State *l)
{
	unstore_callback(l, 2);
	return 0;
}

//==============================================================================

void pi_lua_generic_push(lua_State *l, InputBindings::JoyAxis axis)
{
	if (!axis.Enabled()) {
		lua_pushnil(l);
		return;
	}

	LuaTable axisTab(l);
	axisTab.Set("joystick", axis.joystickId);
	axisTab.Set("axis", axis.axis);
	axisTab.Set("direction", axis.direction);
}

void pi_lua_generic_pull(lua_State *l, int index, InputBindings::JoyAxis &out)
{
	if (!lua_istable(l, index)) {
		out.joystickId = 0;
		out.axis = 0;
		out.direction = 0;
		return;
	}

	LuaTable axisTab(l, index);
	out.joystickId = axisTab.Get<int>("joystick");
	out.axis = axisTab.Get<int>("axis");
	out.direction = axisTab.Get<int>("direction");
}

void pi_lua_generic_push(lua_State *l, InputBindings::KeyBinding bind)
{
	if (bind.type == KeyBinding::Type::Disabled) {
		lua_pushnil(l);
		return;
	}

	LuaTable bindingTable(l);
	if (bind.type == KeyBinding::Type::KeyboardKey) {
		bindingTable.Set("key", bind.keycode);
	} else if (bind.type == KeyBinding::Type::JoystickButton) {
		bindingTable.Set("joystick", bind.joystick.id);
		bindingTable.Set("button", bind.joystick.button);
	} else if (bind.type == KeyBinding::Type::JoystickHat) {
		bindingTable.Set("joystick", bind.joystick.id);
		bindingTable.Set("hat", bind.joystick.hat);
		bindingTable.Set("dir", bind.joystick.button);
	} else {
		bindingTable.Set("mouse", bind.mouse.button);
	}
}

void pi_lua_generic_pull(lua_State *l, int index, InputBindings::KeyBinding &out)
{
	out.type = KeyBinding::Type::Disabled;
	if (!lua_istable(l, index))
		return;

	LuaTable bindingTable(l, index);
	if (bindingTable.Get<bool>("key")) {
		out.type = KeyBinding::Type::KeyboardKey;
		out.keycode = bindingTable.Get<int>("key");
	} else if (bindingTable.Get<bool>("button")) {
		out.type = KeyBinding::Type::JoystickButton;
		out.joystick.id = bindingTable.Get<int>("joystick");
		out.joystick.button = bindingTable.Get<int>("button");
	} else if (bindingTable.Get<bool>("hat")) {
		out.type = KeyBinding::Type::JoystickHat;
		out.joystick.id = bindingTable.Get<int>("joystick");
		out.joystick.hat = bindingTable.Get<int>("hat");
		out.joystick.button = bindingTable.Get<int>("dir");
	} else if (bindingTable.Get<bool>("mouse")) {
		out.type = KeyBinding::Type::MouseButton;
		out.mouse.button = bindingTable.Get<int>("mouse");
	}
}

void pi_lua_generic_push(lua_State *l, InputBindings::KeyChord chord)
{
	LuaTable chordTab(l);
	chordTab.Set("enabled", chord.Enabled());
	if (chord.activator.Enabled())
		chordTab.Set("activator", chord.activator);

	if (chord.modifier1.Enabled())
		chordTab.Set("modifier1", chord.modifier1);

	if (chord.modifier2.Enabled())
		chordTab.Set("modifier2", chord.modifier2);
}

void pi_lua_generic_pull(lua_State *l, int index, InputBindings::KeyChord &out)
{
	if (!lua_istable(l, index)) {
		out = {};
		return;
	}

	LuaTable chordTab(l, index);
	out.activator = chordTab.Get<KeyBinding>("activator");
	out.modifier1 = chordTab.Get<KeyBinding>("modifier1");
	out.modifier2 = chordTab.Get<KeyBinding>("modifier2");
}

static LuaMetaType<LuaInputCallback> s_inputCallback("LuaInputCallback");
static LuaMetaType<LuaInputAction> s_inputActionBinding("LuaInputAction");
static LuaMetaType<LuaInputAxis> s_inputAxisBinding("LuaInputAxis");
static LuaMetaType<LuaJoystickInfo> s_joystickInfoBinding("LuaJoystickInfo");
static LuaMetaType<LuaInputFrame> s_inputFrame("LuaInputFrame");
void LuaInput::Register()
{
	lua_State *l = Lua::manager->GetLuaState();

	LUA_DEBUG_START(l);

	static const luaL_Reg l_methods[] = {
		{ "EnableBindings", l_input_enable_bindings },
		{ "GetBindingPages", l_input_get_binding_pages },
		{ "GetActionBinding", l_input_get_action_binding },
		{ "GetAxisBinding", l_input_get_axis_binding },
		{ "GetKeyName", l_input_get_key_name },
		{ "GetJoystickCount", l_input_get_joystick_count },
		{ "GetJoystickName", l_input_get_joystick_name },
		{ "IsJoystickConnected", l_input_is_joystick_connected },
		{ "GetJoystick", l_input_get_joystick },
		{ "SaveJoystickConfig", l_input_save_joystick_config },
		{ "SaveBinding", l_input_save_binding },
		{ "GetMouseYInverted", l_input_get_mouse_y_inverted },
		{ "SetMouseYInverted", l_input_set_mouse_y_inverted },
		{ "GetJoystickEnabled", l_input_get_joystick_enabled },
		{ "SetJoystickEnabled", l_input_set_joystick_enabled },

		{ "GetMouseCaptured", l_input_get_mouse_captured },

		{ "CreateInputFrame", l_input_create_input_frame },
		{ "RegisterActionBinding", l_input_register_action_binding },
		{ "RegisterAxisBinding", l_input_register_axis_binding },
		{ "DisconnectCallback", l_input_disconnect_callback },

		{ NULL, NULL }
	};

	static const luaL_Reg l_attrs[] = {
		{ NULL, NULL }
	};

	lua_getfield(l, LUA_REGISTRYINDEX, "CoreImports");
	LuaObjectBase::CreateObject(l_methods, l_attrs, 0);

	// Set as CoreImports.Input
	lua_pushvalue(l, -1);
	lua_setfield(l, -3, "Input");

	{
		LuaTable keys(l);
		for (const auto &pair : s_input_keys) {
			keys.Set(pair.first, pair.second);
		}

		lua_setfield(l, -2, "keys");
	}

	lua_pop(l, 2);

	LUA_DEBUG_CHECK(l, 0);

	// Create the binding connection storage table
	lua_newtable(l);
	s_connections = LuaRef(l, -1);
	lua_pop(l, 1);

	LUA_DEBUG_CHECK(l, 0);

	s_inputCallback.CreateMetaType(l);

	s_inputActionBinding.CreateMetaType(l);
	s_inputActionBinding.StartRecording()
		.AddMember("id", &LuaInputAction::id)
		.AddMember("type", &LuaInputAction::getType)
		.AddMember("binding", &LuaInputAction::getBinding, &LuaInputAction::setBinding)
		.AddMember("binding2", &LuaInputAction::getBinding2, &LuaInputAction::setBinding2)
		.AddFunction("SetPressed", &LuaInputAction::setPressed)
		.AddFunction("SetReleased", &LuaInputAction::setReleased)
		.AddFunction("IsActive", &LuaInputAction::isActive)
		.AddFunction("IsJustActive", &LuaInputAction::isJustActive)
		.AddFunction("OnPressed", &LuaInputAction::AddOnPressedCallback)
		.AddFunction("OnReleased", &LuaInputAction::AddOnReleasedCallback);
	s_inputActionBinding.StopRecording();

	s_inputAxisBinding.CreateMetaType(l);
	s_inputAxisBinding.StartRecording()
		.AddMember("id", &LuaInputAxis::id)
		.AddMember("type", &LuaInputAxis::getType)
		.AddMember("axis", &LuaInputAxis::getAxisBinding, &LuaInputAxis::setAxisBinding)
		.AddMember("positive", &LuaInputAxis::getPositive, &LuaInputAxis::setPositive)
		.AddMember("negative", &LuaInputAxis::getNegative, &LuaInputAxis::setNegative)
		.AddFunction("SetValue", &LuaInputAxis::setValue)
		.AddFunction("GetValue", &LuaInputAxis::getValue)
		.AddFunction("OnValueChanged", &LuaInputAxis::AddValueCallback);
	s_inputAxisBinding.StopRecording();

	s_joystickInfoBinding.CreateMetaType(l);
	s_joystickInfoBinding.StartRecording()
		.AddMember("name", &LuaJoystickInfo::getName)
		.AddMember("numButtons", &LuaJoystickInfo::numButtons)
		.AddMember("numHats", &LuaJoystickInfo::numHats)
		.AddMember("numAxes", &LuaJoystickInfo::numAxes)
		.AddFunction("GetButtonState", &LuaJoystickInfo::getButtonState)
		.AddFunction("GetHatState", &LuaJoystickInfo::getHatState)
		.AddFunction("GetAxisValue", &LuaJoystickInfo::getAxisValue)
		.AddFunction("GetAxisDeadzone", &LuaJoystickInfo::getAxisDeadzone)
		.AddFunction("SetAxisDeadzone", &LuaJoystickInfo::setAxisDeadzone)
		.AddFunction("GetAxisCurve", &LuaJoystickInfo::getAxisCurve)
		.AddFunction("SetAxisCurve", &LuaJoystickInfo::setAxisCurve)
		.AddFunction("GetAxisZeroToOne", &LuaJoystickInfo::getAxisZeroToOne)
		.AddFunction("SetAxisZeroToOne", &LuaJoystickInfo::setAxisZeroToOne);
	s_joystickInfoBinding.StopRecording();

	s_inputFrame.CreateMetaType(l);
	s_inputFrame.StartRecording()
		.AddMember("id", &LuaInputFrame::getId)
		.AddFunction("AddToStack", &LuaInputFrame::addToStack)
		.AddFunction("RemoveFromStack", &LuaInputFrame::removeFromStack)
		.AddFunction("AddAction", &LuaInputFrame::addAction)
		.AddFunction("AddAxis", &LuaInputFrame::addAxis);
	s_inputFrame.StopRecording();

	LUA_DEBUG_END(l, 0);
}

void LuaInput::Uninit()
{
	lua_State *l = s_connections.GetLua();

	LUA_DEBUG_START(l);

	s_connections.PushCopyToStack();

	// Clear all connection tickets before Lua is shut down
	lua_pushnil(l);
	while (lua_next(l, -2)) {
		if (lua_isfunction(l, -2)) {

			lua_pushnil(l);
			while (lua_next(l, -2)) {
				auto *cb = LuaPull<LuaInputCallback *>(l, -1);
				cb->connection.m_connection.disconnect();
				lua_pop(l, 1);
			}

		}

		lua_pop(l, 1);
	}

	lua_pop(l, 1);
	s_connections.Unref();

	LUA_DEBUG_END(l, 0);
}
