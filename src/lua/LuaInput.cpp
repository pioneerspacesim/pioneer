// Copyright © 2008-2020 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#include "LuaInput.h"
#include "GameConfig.h"
#include "Input.h"
#include "InputBindings.h"
#include "Lang.h"
#include "LuaObject.h"
#include "LuaUtils.h"
#include "LuaWrappable.h"
#include "Pi.h"
#include "src/lua.h"

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
 * >			{ -- a binding
 * >				type = 'action', -- the type of binding; can be 'action' or 'axis'
 * >				id = 'BindToggleLuaConsole' -- the internal ID of the binding; used as a translation key and passed to Input.SetKeyBinding
 * >			},
 * >			{ -- an axis binding
 * >				type = 'axis',
 * >				id = 'BindAxisPitch'
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
				if (type.second == Input::BindingGroup::EntryType::ENTRY_ACTION) {
					setup_binding_table(l, type.first.c_str(), "action");
				} else {
					setup_binding_table(l, type.first.c_str(), "axis");
				}

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

#if 0
static int l_input_set_action_binding(lua_State *l)
{
	const char *binding_id = luaL_checkstring(l, 1);
	const char *binding_config_1 = lua_tostring(l, 2);
	const char *binding_config_2 = lua_tostring(l, 3);
	KeyBindings::ActionBinding *action = Pi::input->GetActionBinding(binding_id);

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
	KeyBindings::AxisBinding *binding = Pi::input->GetAxisBinding(binding_id);

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
#endif

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

static void pi_lua_generic_push(lua_State *l, InputBindings::Action *action)
{
}

static void pi_lua_generic_push(lua_State *l, InputBindings::Axis *axis)
{
}

void LuaInput::Register()
{
	lua_State *l = Lua::manager->GetLuaState();

	LUA_DEBUG_START(l);

	static const luaL_Reg l_methods[] = {
		{ "EnableBindings", l_input_enable_bindings },
		{ "GetBindingPages", l_input_get_binding_pages },
#if 0 // FIXME: actually implement these!
		{ "GetActionBinding", l_input_get_action_binding },
		{ "AddActionBinding", l_input_add_action_binding },
		{ "GetAxisBinding", l_input_get_axis_binding },
		{ "AddAxisBinding", l_input_add_axis_binding },
#endif
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
