// Copyright © 2008-2020 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#include "LuaInput.h"
#include "GameConfig.h"
#include "Input.h"
#include "KeyBindings.h"
#include "Lang.h"
#include "LuaObject.h"
#include "LuaUtils.h"
#include "Pi.h"
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

static void push_key_binding(lua_State *l, KeyBindings::KeyBinding *kb, const char *binding, const char *description)
{
	if (kb->Enabled()) {
		lua_pushstring(l, kb->ToString().c_str());
		lua_setfield(l, -2, binding);
		lua_pushstring(l, kb->Description().c_str());
		lua_setfield(l, -2, description);
	}
}
/*
 * Function: GetBindings
 *
 * Get a table listing all the current key and axis bindings.
 *
 * > bindings = Input.GetBindings()
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
 * >		{ -- a group
 * >			id = 'Miscellaneous', -- the translation key of the name of the group
 * >			{ -- a binding
 * >				type = 'action', -- the type of binding; can be 'action' or 'axis'
 * >				id = 'BindToggleLuaConsole', -- the internal ID of the binding; used as a translation key and passed to Input.SetKeyBinding
 * >				binding1 = 'Key96', -- the first bound key or axis (value stored in config file)
 * >				bindingDescription1 = '`', -- display text for the first bound key or axis
 * >				binding2 = 'Key96', -- the second bound key or axis (value stored in config file)
 * >				bindingDescription2 = '`', -- display text for the second bound key or axis
 * >			},
 * >			{ -- an axis binding
 * >				type = 'axis',
 * >				id = 'BindAxisPitch',
 * >				axis = 'Joy[UUID]/Axis3/DZ0.0/E1.0', -- The joystick binding (value stored in the config file)
 * >				positive = 'Key96', -- the key bound to the positive half of the axis
 * >				positiveDescription = '`', -- as normal for key bindings
 * >				negative = 'Key96', -- the key bound to the negative half of the axis
 * >				negativeDescription = '`', -- as normal for key bindings
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
static int l_input_get_bindings(lua_State *l)
{
	LUA_DEBUG_START(l);

	lua_newtable(l); // [-1] bindings
	using namespace KeyBindings;

	int page_idx = 1;
	for (auto page : Pi::input.GetBindingPages()) {
		lua_pushunsigned(l, page_idx++);
		setup_binding_table(l, page.first.c_str(), "page");

		int group_idx = 1;
		for (auto group : page.second.groups) {
			lua_pushunsigned(l, group_idx++);
			setup_binding_table(l, group.first.c_str(), "group");

			int binding_idx = 1;
			for (auto type : group.second.bindings) {
				lua_pushunsigned(l, binding_idx++);
				if (type.second == Input::BindingGroup::EntryType::ENTRY_ACTION) {
					ActionBinding *ab = Pi::input.GetActionBinding(type.first);
					if (!ab) continue; // Should never happen, but include it here for future proofing.
					setup_binding_table(l, type.first.c_str(), "action");

					push_key_binding(l, &ab->binding1, "binding1", "bindingDescription1");
					push_key_binding(l, &ab->binding2, "binding2", "bindingDescription2");
				} else {
					AxisBinding *ab = Pi::input.GetAxisBinding(type.first);
					if (!ab) continue; // Should never happen, but include it here for future proofing.
					setup_binding_table(l, type.first.c_str(), "axis");

					if (ab->axis.Enabled()) {
						lua_pushstring(l, ab->axis.ToString().c_str());
						lua_setfield(l, -2, "axis");
						lua_pushstring(l, ab->axis.Description().c_str());
						lua_setfield(l, -2, "axisDescription");
					}

					push_key_binding(l, &ab->positive, "positive", "positiveDescription");
					push_key_binding(l, &ab->negative, "negative", "negativeDescription");
				}

				// [-3] group, [-2] idx, [-1] binding
				lua_settable(l, -3);
			}

			// [-3] page, [-2] idx, [-1] group
			lua_settable(l, -3);
		}

		// [-3] bindings, [-2] idx, [-1] group
		lua_settable(l, -3);
	}

	LUA_DEBUG_END(l, 1);
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

static int l_input_set_action_binding(lua_State *l)
{
	const char *binding_id = luaL_checkstring(l, 1);
	const char *binding_config_1 = lua_tostring(l, 2);
	const char *binding_config_2 = lua_tostring(l, 3);
	KeyBindings::ActionBinding *action = Pi::input.GetActionBinding(binding_id);

	KeyBindings::KeyBinding kb1, kb2;
	if (binding_config_1) {
		if (!KeyBindings::KeyBinding::FromString(binding_config_1, kb1))
			return luaL_error(l, "invalid first key binding given to Input.SetKeyBinding");
	} else
		kb1.Clear();
	if (binding_config_2) {
		if (!KeyBindings::KeyBinding::FromString(binding_config_2, kb2))
			return luaL_error(l, "invalid second key binding given to Input.SetKeyBinding");
	} else
		kb2.Clear();
	action->binding1 = kb1;
	action->binding2 = kb2;
	Pi::config->SetString(binding_id, action->ToString());
	Pi::config->Save();
	return 0;
}

static int l_input_set_axis_binding(lua_State *l)
{
	const char *binding_id = luaL_checkstring(l, 1);
	const char *binding_config_axis = lua_tostring(l, 2);
	const char *binding_config_positive = lua_tostring(l, 3);
	const char *binding_config_negative = lua_tostring(l, 4);
	KeyBindings::AxisBinding *binding = Pi::input.GetAxisBinding(binding_id);

	KeyBindings::JoyAxisBinding ab;
	if (binding_config_axis) {
		if (!KeyBindings::JoyAxisBinding::FromString(binding_config_axis, ab))
			return luaL_error(l, "invalid axis binding given to Input.SetKeyBinding");
	} else
		ab.Clear();

	KeyBindings::KeyBinding kb1, kb2;
	if (binding_config_positive) {
		if (!KeyBindings::KeyBinding::FromString(binding_config_positive, kb1))
			return luaL_error(l, "invalid first key binding given to Input.SetKeyBinding");
	} else
		kb1.Clear();
	if (binding_config_negative) {
		if (!KeyBindings::KeyBinding::FromString(binding_config_negative, kb2))
			return luaL_error(l, "invalid second key binding given to Input.SetKeyBinding");
	} else
		kb2.Clear();

	binding->axis = ab;
	binding->positive = kb1;
	binding->negative = kb2;
	Pi::config->SetString(binding_id, binding->ToString());
	Pi::config->Save();
	return 0;
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

void LuaInput::Register()
{
	lua_State *l = Lua::manager->GetLuaState();

	LUA_DEBUG_START(l);

	static const luaL_Reg l_methods[] = {
		{ "EnableBindings", l_input_enable_bindings },
		{ "DisableBindings", l_input_disable_bindings },
		{ "GetBindings", l_input_get_bindings },
		{ "SetActionBinding", l_input_set_action_binding },
		{ "SetAxisBinding", l_input_set_axis_binding },
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
