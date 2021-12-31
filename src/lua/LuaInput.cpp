// Copyright © 2008-2022 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#include "LuaInput.h"
#include "GameConfig.h"
#include "Input.h"
#include "InputBindings.h"
#include "Lang.h"
#include "LuaMetaType.h"
#include "LuaObject.h"
#include "LuaUtils.h"
#include "LuaWrappable.h"
#include "Pi.h"

#include "SDL_keyboard.h"
#include <sstream>

using namespace InputBindings;

// Proxy object for an action binding
// TODO: push the actual action pointer to lua
struct LuaInputAction : public LuaWrappable {
	LuaInputAction(std::string _id) :
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

	Action *getAction() const { return Pi::input->GetActionBinding(id); }
};

// Proxy object for an axis binding
// TODO: push the actual axis pointer to lua
struct LuaInputAxis : public LuaWrappable {
	LuaInputAxis(std::string _id) :
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

	Axis *getAxis() const { return Pi::input->GetAxisBinding(id); }
};

template <typename T>
static int lua_deleter(lua_State *l)
{
	T *ref = LuaPull<T *>(l, 1);
	ref->~T();
	return 0;
}

#define GENERIC_COPY_OBJ_DEF(Typename)                                      \
	template <>                                                             \
	const char *LuaObject<Typename>::s_type = #Typename;                    \
	inline void pi_lua_generic_pull(lua_State *l, int index, Typename &out) \
	{                                                                       \
		assert(l == Lua::manager->GetLuaState());                           \
		out = *LuaObject<Typename>::CheckFromLua(index);                    \
	}                                                                       \
	inline void pi_lua_generic_push(lua_State *l, const Typename &value)    \
	{                                                                       \
		assert(l == Lua::manager->GetLuaState());                           \
		LuaObject<Typename>::PushToLua(value);                              \
	}

GENERIC_COPY_OBJ_DEF(LuaInputAction)
GENERIC_COPY_OBJ_DEF(LuaInputAxis)

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
	if (!lua_istable(l, index))
		return;

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
	if (!lua_istable(l, index)) return;
	LuaTable chordTab(l, index);
	if (!chordTab.Get<bool>("enabled"))
		return;

	out.activator = chordTab.Get<KeyBinding>("activator");
	out.modifier1 = chordTab.Get<KeyBinding>("modifier1");
	out.modifier2 = chordTab.Get<KeyBinding>("modifier2");
}

static LuaMetaType<LuaInputAction> s_inputActionBinding("LuaInputAction");
static LuaMetaType<LuaInputAxis> s_inputAxisBinding("LuaInputAxis");
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
		{ "GetJoystickName", l_input_get_joystick_name },
		{ "SaveBinding", l_input_save_binding },
		{ "GetMouseYInverted", l_input_get_mouse_y_inverted },
		{ "SetMouseYInverted", l_input_set_mouse_y_inverted },
		{ "GetJoystickEnabled", l_input_get_joystick_enabled },
		{ "SetJoystickEnabled", l_input_set_joystick_enabled },
		{ NULL, NULL }
	};

	static const luaL_Reg l_attrs[] = {
		{ NULL, NULL }
	};

	lua_getfield(l, LUA_REGISTRYINDEX, "CoreImports");
	LuaObjectBase::CreateObject(l_methods, l_attrs, 0);
	lua_setfield(l, -2, "Input");
	lua_pop(l, 1);

	s_inputActionBinding.CreateMetaType(l);
	s_inputActionBinding.StartRecording()
		.AddMember("id", &LuaInputAction::id)
		.AddMember("type", &LuaInputAction::getType)
		.AddMember("binding", &LuaInputAction::getBinding, &LuaInputAction::setBinding)
		.AddMember("binding2", &LuaInputAction::getBinding2, &LuaInputAction::setBinding2);

	lua_pushcfunction(l, lua_deleter<LuaInputAction>);
	lua_setfield(l, -2, "__gc");
	s_inputActionBinding.StopRecording();

	s_inputAxisBinding.CreateMetaType(l);
	s_inputAxisBinding.StartRecording()
		.AddMember("id", &LuaInputAxis::id)
		.AddMember("type", &LuaInputAxis::getType)
		.AddMember("axis", &LuaInputAxis::getAxisBinding, &LuaInputAxis::setAxisBinding)
		.AddMember("positive", &LuaInputAxis::getPositive, &LuaInputAxis::setPositive)
		.AddMember("negative", &LuaInputAxis::getNegative, &LuaInputAxis::setNegative);
	lua_pushcfunction(l, lua_deleter<LuaInputAxis>);
	lua_setfield(l, -2, "__gc");
	s_inputAxisBinding.StopRecording();

	LUA_DEBUG_END(l, 0);
}
