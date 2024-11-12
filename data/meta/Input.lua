-- Copyright © 2008-2024 Pioneer Developers. See AUTHORS.txt for details
-- Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

-- This file implements type information about C++ classes for Lua static analysis
-- TODO: document function usage

---@meta

---@class Input
local Input = {}

--- List of input keys to be used with various input functions.
--- The list is a mostly-complete port of common SDL_KeyCode values, indexed by
--- lowercase key names.
---@type table<string, integer>
Input.keys = {}

--==================================

---@class Input.ActionBinding
---@field id string
---@field type string
---@field binding table
---@field binding2 table
local ActionBinding = {}

function ActionBinding:OnPress() end
function ActionBinding:OnRelease() end

---@return boolean
function ActionBinding:IsActive() end
---@return boolean
function ActionBinding:IsJustActive() end

--==================================

---@class Input.AxisBinding
---@field id string
---@field type string
---@field axis table
---@field positive table
---@field negative table
local AxisBinding = {}

---@param val number
function AxisBinding:SetValue(val) end
---@return number
function AxisBinding:GetValue() end

--==================================

---@class Input.JoystickInfo
---@field name string
---@field numButtons integer
---@field numHats integer
---@field numAxes integer
local JoystickInfo = {}

---@param button integer
---@return boolean
function JoystickInfo:GetButtonState(button) end

---@param hat integer
---@return boolean
function JoystickInfo:GetHatState(hat) end

---@param axis integer
---@return number
function JoystickInfo:GetAxisValue(axis) end

---@param axis integer
---@return number
function JoystickInfo:GetAxisDeadzone(axis) end

---@param axis integer
---@param deadzone number
function JoystickInfo:SetAxisDeadzone(axis, deadzone) end

---@param axis integer
---@return number
function JoystickInfo:GetAxisCurve(axis) end

---@param axis integer
---@param curve number
function JoystickInfo:SetAxisCurve(axis, curve) end

---@param axis integer
---@return boolean
function JoystickInfo:GetAxisZeroToOne(axis) end
---@param axis integer
---@param zeroToOne boolean
function JoystickInfo:SetAxisZeroToOne(axis, zeroToOne) end

--==================================

---@class Input.InputFrame
---@field id string
local InputFrame = {}

-- Add the given action binding to this frame
---@param action Input.ActionBinding
function InputFrame:AddAction(action) end

-- Add the given axis binding to this frame
---@param axis Input.AxisBinding
function InputFrame:AddAxis(axis) end

-- Add the input frame to the stack of active inputs
---@return boolean
function InputFrame:AddToStack() end

-- Remove the input frame from the stack of active inputs
function InputFrame:RemoveFromStack() end

--==================================

---@param enable boolean
function Input.EnableBindings(enable) end

---@return (Input.AxisBinding|Input.ActionBinding)[][][]
function Input.GetBindingPages() end

---@param id string
---@return Input.ActionBinding?
function Input.GetActionBinding(id) end

---@param id string
---@return Input.AxisBinding?
function Input.GetAxisBinding(id) end

---@param key integer
function Input.GetKeyName(key) end

---@return number
function Input.GetJoystickCount() end

---@param joy integer
---@return string
function Input.GetJoystickName(joy) end

---@param joy integer
---@return boolean
function Input.IsJoystickConnected(joy) end

---@param joy integer
---@return Input.JoystickInfo?
function Input.GetJoystick(joy) end

---@param joy integer
function Input.SaveJoystickConfig(joy) end

---@param binding Input.AxisBinding | Input.ActionBinding
function Input.SaveBinding(binding) end

---@return boolean
function Input.GetMouseYInverted() end

---@param inverted boolean
function Input.SetMouseYInverted(inverted) end

---@return boolean
function Input.GetJoystickEnabled() end

---@param enabled boolean
function Input.SetJoystickEnabled(enabled) end

---@return boolean
function Input.GetMouseCaptured() end

-- Create an InputFrame to add action/axis bindings to the active input stack
---@param id string The debug identifier of this InputFrame
---@param modal boolean? Should this InputFrame be modal?
---@return Input.InputFrame
function Input.CreateInputFrame(id, modal) end

return Input
