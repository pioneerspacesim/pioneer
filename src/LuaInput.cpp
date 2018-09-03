// Copyright © 2008-2018 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#include "LuaInput.h"
#include "LuaUtils.h"
#include "LuaObject.h"
#include "Lang.h"
#include "Input.h"
#include "KeyBindings.h"
#include "Pi.h"
/*
 * Interface: Input
 *
 * A global table that exposes the engine's Input interface. Key bindings and
 * state can be queried here.
 *
 */

static void push_bindings(lua_State *l, const KeyBindings::BindingPrototype *protos) {
	LUA_DEBUG_START(l);

	lua_newtable(l); // [-1] bindings
	lua_pushnil(l); // [-2] bindings, [-1] group (no current group)

	assert(!protos[0].function); // first entry should be a group header

	int group_idx = 1;
	int binding_idx = 1;
	for (const KeyBindings::BindingPrototype *proto = protos; proto->label; ++proto) {
		if (! proto->function) {
			// start a new named binding group

			// [-2] bindings, [-1] group
			lua_pop(l, 1);
			// [-1] bindings
			lua_newtable(l);
			lua_pushstring(l, proto->label);
			lua_setfield(l, -2, "label");
			// [-2] bindings, [-1] group
			lua_pushvalue(l, -1);
			// [-3] bindings, [-2] group, [-1] group copy
			lua_rawseti(l, -3, group_idx);
			++group_idx;

			binding_idx = 1;
		} else {
			// key or axis binding prototype

			// [-2] bindings, [-1] group
			lua_createtable(l, 0, 5);
			// [-3] bindings, [-2] group, [-1] binding

			// fields are: type ('KEY' or 'AXIS'), id ('BindIncreaseSpeed'), label ('Increase Speed'), binding ('Key13'), bindingDescription ('')
			lua_pushstring(l, (proto->kb ? "KEY" : "AXIS"));
			lua_setfield(l, -2, "type");
			lua_pushstring(l, proto->function);
			lua_setfield(l, -2, "id");
			lua_pushstring(l, proto->label);
			lua_setfield(l, -2, "label");
			if (proto->kb) {
				const KeyBindings::KeyBinding kb1 = proto->kb->binding1;
				if (kb1.Enabled()) {
					lua_pushstring(l, kb1.ToString().c_str());
					lua_setfield(l, -2, "binding1");
					lua_pushstring(l, kb1.Description().c_str());
					lua_setfield(l, -2, "bindingDescription1");
				}
				const KeyBindings::KeyBinding kb2 = proto->kb->binding2;
				if (kb2.Enabled()) {
					lua_pushstring(l, kb2.ToString().c_str());
					lua_setfield(l, -2, "binding2");
					lua_pushstring(l, kb2.Description().c_str());
					lua_setfield(l, -2, "bindingDescription2");
				}
			} else if (proto->ab) {
				const KeyBindings::JoyAxisBinding &ab = *proto->ab;
				lua_pushstring(l, ab.ToString().c_str());
				lua_setfield(l, -2, "binding1");
				lua_pushstring(l, ab.Description().c_str());
				lua_setfield(l, -2, "bindingDescription1");
			} else {
				assert(0); // invalid prototype binding
			}

			// [-3] bindings, [-2] group, [-1] binding
			lua_rawseti(l, -2, binding_idx);
			++binding_idx;
		}

		LUA_DEBUG_CHECK(l, 2); // [-2] bindings, [-1] group
	}

	// pop the group table (which should already have been put in the bindings table)
	lua_pop(l, 1);

	LUA_DEBUG_END(l, 1);
}

/*
 * Function: GetKeyBindings
 *
 * Get a table listing all the current key and axis bindings.
 *
 * > bindings = Input.GetKeyBindings()
 *
 * Returns:
 *
 *   bindings - A table containing all the key and axis bindings.
 *
 * The bindings table has the following structure (in Lua syntax):
 *
 * > bindings = {
 * >   { -- a page
 * >      label = 'CONTROLS', -- the (translated) name of the page
 * >      { -- a group
 * >          label = 'Miscellaneous', -- the (translated) name of the group
 * >          { -- a binding
 * >              type = 'KEY', -- the type of binding; can be 'KEY' or 'AXIS'
 * >              id = 'BindToggleLuaConsole', -- the internal ID of the binding; pass this to Engine.SetKeyBinding
 * >              label = 'Toggle Lua console', -- the (translated) label for the binding
 * >              binding1 = 'Key96', -- the first bound key or axis (value stored in config file)
 * >              bindingDescription1 = '`', -- display text for the first bound key or axis
 * >              binding2 = 'Key96', -- the second bound key or axis (value stored in config file)
 * >              bindingDescription2 = '`', -- display text for the second bound key or axis
 * >          },
 * >          -- ... more bindings
 * >      },
 * >      -- ... more groups
 * >   },
 * >   -- ... more pages
 * > }
 *
 * Availability:
 *
 *   October 2013
 *
 * Status:
 *
 *   temporary
 */
static int l_input_get_key_bindings(lua_State *l)
{
	// XXX maybe this key-bindings table should be cached in the Lua registry?

	int idx = 1;
	lua_newtable(l);

#define BINDING_PAGE(name) \
	push_bindings(l, KeyBindings :: BINDING_PROTOS_ ## name); \
	lua_pushstring(l, Lang :: name); \
	lua_setfield(l, -2, "label"); \
	lua_rawseti(l, -2, idx++);
#include "KeyBindings.inc.h"

	return 1;
}

static int l_input_enable_bindings(lua_State *l)
{
	KeyBindings::EnableBindings();
	return 0;
}

static int l_input_disable_bindings(lua_State *l)
{
	KeyBindings::DisableBindings();
	return 0;
}

static int set_key_binding(lua_State *l, const char *config_id, KeyBindings::ActionBinding *action) {
	const char *binding_config_1 = lua_tostring(l, 2);
	const char *binding_config_2 = lua_tostring(l, 3);
	KeyBindings::KeyBinding kb1, kb2;
	if (binding_config_1) {
		if (!KeyBindings::KeyBinding::FromString(binding_config_1, kb1))
			return luaL_error(l, "invalid first key binding given to Engine.SetKeyBinding");
	} else
		kb1.Clear();
	if (binding_config_2) {
		if (!KeyBindings::KeyBinding::FromString(binding_config_2, kb2))
			return luaL_error(l, "invalid second key binding given to Engine.SetKeyBinding");
	} else
		kb2.Clear();
	action->binding1 = kb1;
	action->binding2 = kb2;
	Pi::config->SetString(config_id, action->ToString());
	Pi::config->Save();
	return 0;
}

static int set_axis_binding(lua_State *l, const char *config_id, KeyBindings::JoyAxisBinding *binding) {
	const char *binding_config = lua_tostring(l, 2);
	KeyBindings::JoyAxisBinding ab;
	if (binding_config) {
		if (!KeyBindings::JoyAxisBinding::FromString(binding_config, ab))
			return luaL_error(l, "invalid axis binding given to Engine.SetKeyBinding");
	} else
		ab.Clear();
	*binding = ab;
	Pi::config->SetString(config_id, ab.ToString());
	Pi::config->Save();
	return 0;
}

static int l_input_set_key_binding(lua_State *l)
{
	const char *binding_id = luaL_checkstring(l, 1);

#define KEY_BINDING(action, config_id, label, def1, def2) \
	if (strcmp(binding_id, config_id) == 0) { return set_key_binding(l, config_id, &KeyBindings :: action); }
#define AXIS_BINDING(action, config_id, label, default_axis) \
	if (strcmp(binding_id, config_id) == 0) { return set_axis_binding(l, config_id, &KeyBindings :: action); }

#include "KeyBindings.inc.h"

	return luaL_error(l, "Invalid binding ID given to Engine.SetKeyBinding");
}

static int l_input_get_mouse_y_inverted(lua_State *l)
{
	lua_pushboolean(l, Pi::config->Int("InvertMouseY") != 0);
	return 1;
}

static int l_input_set_mouse_y_inverted(lua_State *l)
{
	if (lua_isnone(l, 1))
		return luaL_error(l, "SetMouseYInverted takes one boolean argument");
	const bool inverted = lua_toboolean(l, 1);
	Pi::config->SetInt("InvertMouseY", (inverted ? 1 : 0));
	Pi::config->Save();
	Pi::input.SetMouseYInvert(inverted);
	return 0;
}

static int l_input_get_joystick_enabled(lua_State *l)
{
	lua_pushboolean(l, Pi::config->Int("EnableJoystick") != 0);
	return 1;
}

static int l_input_set_joystick_enabled(lua_State *l)
{
	if (lua_isnone(l, 1))
		return luaL_error(l, "SetJoystickEnabled takes one boolean argument");
	const bool enabled = lua_toboolean(l, 1);
	Pi::config->SetInt("EnableJoystick", (enabled ? 1 : 0));
	Pi::config->Save();
	Pi::input.SetJoystickEnabled(enabled);
	return 0;
}

void LuaInput::Register() {
    lua_State *l = Lua::manager->GetLuaState();

	LUA_DEBUG_START(l);

    static const luaL_Reg l_methods[] = {
        { "EnableBindings", l_input_enable_bindings },
		{ "DisableBindings", l_input_disable_bindings },
		{ "GetKeyBindings", l_input_get_key_bindings },
		{ "SetKeyBinding", l_input_set_key_binding },
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

	LUA_DEBUG_END(l, 0);
}
