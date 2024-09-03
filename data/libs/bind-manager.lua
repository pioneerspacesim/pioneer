-- Copyright © 2008-2024 Pioneer Developers. See AUTHORS.txt for details
-- Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

local bindManager = {}
local Input = require 'Input'
local linput = require 'Lang'.GetResource("input-core")
local lc = require 'Lang'.GetResource("core")

local axis_names = { linput.X, linput.Y, linput.Z, linput.RX, linput.RY, linput.RZ }
local mouse_names = { linput.MOUSE_LMB, linput.MOUSE_MMB, linput.MOUSE_RMB }

function bindManager.getAxisName(axis)
	return linput.AXIS .. (axis_names[axis + 1] or tostring(axis))
end

--
-- Function: bindManager.getBindingDesc
--
-- Returns a localized string with the name of the given key or axis
--
-- Example:
--
-- > my_bind = bindManager.getBindingDesc(keybinding)
--
-- Parameters:
--
--   keybinding - a table generated from InputBinding::KeyBinding, see LuaInput.cpp
--
-- Returns:
--
--   string
--
function bindManager.getBindingDesc(bind)
	if not bind then return end
	if bind.key then
		return Input.GetKeyName(bind.key)
	elseif bind.joystick and bind.hat then
		return bindManager.joyAcronym(bind.joystick) .. linput.HAT .. bind.hat .. linput.DIRECTION .. bind.dir
	elseif bind.joystick and bind.button then
		return bindManager.joyAcronym(bind.joystick) .. linput.BUTTON .. bind.button
	elseif bind.joystick and bind.axis then
		return (bind.direction < 0 and "-" or "") .. bindManager.joyAcronym(bind.joystick) .. bindManager.getAxisName(bind.axis)
	elseif bind.mouse then
		return linput.MOUSE .. (mouse_names[bind.mouse] or tostring(bind.mouse))
	end
end

--
-- Function: bindManager.getChordDesc
--
-- Get a localized name for a key chord to display on a binding
--
-- Example:
--
-- > local binding_string = bindManager.getChordDesc(action_binding)
--
-- Parameters:
--
--   action_binding - a LuaInputAction binding (see GetBindingPages)
--
-- Returns:
--
--   string, some key combination
--
function bindManager.getChordDesc(chord)
	if not chord.enabled then return "" end

	local str = bindManager.getBindingDesc(chord.activator)
	local mod1 = chord.modifier1
	local mod2 = chord.modifier2

	if mod1 then str = bindManager.getBindingDesc(mod1) .. " + " .. str end
	if mod2 then str = bindManager.getBindingDesc(mod2) .. " + " .. str end

	return str
end

--
-- Function: bindManager.localizeBindingId
--
-- Convert an axis or action binding style ID to a translation resource
-- identifier, and get by this identifier a localized string from the
-- "input-core" module. For example, from the string 'BindSomeName', this
-- function will build 'BIND_SOME_NAME'. Based on this, you should create the
-- corresponding items in json file.
--
-- Example:
--
-- > local tooltip = bindManager.localizeBindingId(binding_name)
--
-- Parameters:
--
--   binding_name - string, axis or action binding ID
--
-- Returns:
--
--   table:
--     action
--
--   action_binding - a LuaInputAction binding (see GetBindingPages)
--
function bindManager.localizeBindingId(str)
	local jsonIndex = str:gsub("([^A-Z0-9_])([A-Z0-9])", "%1_%2"):upper()
	return linput[jsonIndex]
end

--
-- Function: bindManager.joyAcronym
--
-- Returns a string with the acronym for given joystick's ID,
--
-- Example:
--
-- > ui.text("  " .. bindManager.joyAcronym(joystickID) .. ":")
--
-- Parameters:
--
--   id - number, KeyBinding.joystick
--
-- Returns:
--
--   a string, "Joy1","Joy2" etc.
--
function bindManager.joyAcronym(id)
	if id == 65535 then return lc.UNKNOWN end
	return linput.JOY .. tostring(id)
end

-- the bindings saved in this table will be automatically updated if they are
-- changed in the controls settings
local bindings = {}

-- Put into the provided table the 'LuaInputAction' object and a tooltip
-- containing the localized name of the action and a key chord.
local function createActionBinding(ref, actionName)
	local action = Input.GetActionBinding(actionName)
	local binding = bindManager.getChordDesc(action.binding)
	local tooltip = bindManager.localizeBindingId(actionName)
	if binding ~= "" then
		tooltip = tooltip .. " (" .. binding .. ")"
	end
	ref.action = action
	ref.tooltip = tooltip
end

-- Put into the provided table the 'LuaInputAxis' object and a tooltip
-- containing the localized name of the axis and a key chord.
local function createAxisBinding(ref, axisName)
	local axis = Input.GetAxisBinding(axisName)
	local positive = bindManager.getChordDesc(axis.positive)
	local negative = bindManager.getChordDesc(axis.negative)
	local tooltip = bindManager.localizeBindingId(axisName)
	if positive ~= "" and negative ~= "" then
		tooltip = tooltip .. " (" .. negative .. "/" .. positive .. ")"
	end
	ref.axis = axis
	ref.tooltip = tooltip
end

--
-- Function: utils.registerAction
--
-- By the name of the action create a table containing the 'LuaInputAction'
-- object and a tooltip containing the localized name of the action and a key
-- chord. This can then be used to draw a button with the corresponding tooltip
-- and trigger an action when the button is clicked
-- This value will be automatically updated when the binding is changed in the
-- settings.
--
-- Example:
--
-- > bindings.limiter = bindManager.registerAction('BindToggleSpeedLimiter')
--
-- Parameters:
--
-- actionName - string, action binding ID
--
-- Returns:
--
--   table:
--     action - a LuaInputAction binding (see GetBindingPages)
--     tooltip - localized string
--
function bindManager.registerAction(actionName)
	if bindings[actionName] then return bindings[actionName] end
	local ref = {}
	createActionBinding(ref, actionName)
	bindings[actionName] = ref
	return ref
end

--
-- Function: bindManager.registerAxis
--
-- By the name of the axis create a table containing the 'LuaInputAxis'
-- object and a tooltip containing the localized name of the axis and a key
-- chord. This can then be used to draw a button with the corresponding tooltip
-- and activate an axis when the button is clicked.
-- This value will be automatically updated when the binding is changed in the
-- settings.
--
-- Example:
--
-- > bindings.yaw = bindManager.registerAxis('BindAxisYaw')
--
-- Parameters:
--
-- axisName - string, axis binding ID
--
-- Returns:
--
--   table:
--     axis - a LuaInputAxis binding (see GetBindingPages)
--     tooltip - localized string
--
function bindManager.registerAxis(axisName)
	if bindings[axisName] then return bindings[axisName] end
	local ref = {}
	createAxisBinding(ref, axisName)
	bindings[axisName] = ref
	return ref
end

-- used by the binding dialog in the settings
function bindManager.updateBinding(id)
	if not bindings[id] then return end
	createActionBinding(bindings[id], id)
end

return bindManager
