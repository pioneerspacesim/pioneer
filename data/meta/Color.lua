-- Copyright © 2008-2023 Pioneer Developers. See AUTHORS.txt for details
-- Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

-- This file implements type information about C++ classes for Lua static analysis

---@meta

---@class Color
---@field r integer
---@field g integer
---@field b integer
---@field a integer
---
---@operator add: Color
---@operator add(number): Color
---@operator mul: Color
---@operator mul(number): Color
---@operator call: Color

---@class Color
local Color = {}

---@param r number
---@param g number
---@param b number
---@param a number?
---@return Color
---@overload fun(hex: string): Color
function _G.Color(r, g, b, a) end

-- Return a copy of this color darkened by the given amount (in 0.0..1.0)
-- Use negative values to lighten.
--
-- Rturns a new Color object
---@param s number
---@return Color
function Color:shade(s) end

-- Increase the perceptual saturation of this color by some factor (in 0.0..1.0)
--
-- Returns a new Color object
---@param t number
---@return Color
function Color:tint(t) end

-- Set the opacity of this color to the passed value (in 0.0..1.0 or 1..255)
--
-- Returns a new Color object
---@param o number
---@return Color
function Color:opacity(o) end
