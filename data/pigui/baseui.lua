-- Copyright © 2008-2024 Pioneer Developers. See AUTHORS.txt for details
-- Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

local Engine = require 'Engine'
local pigui = Engine.pigui

---@class ui
local ui = require 'pigui.libs.forwarded'

ui.rescaleUI = require 'pigui.libs.rescale-ui'

---@type EventQueue
ui.Events = pigui.event_queue

--
-- Function: ui.rescaleFraction
--
-- Smoothly rescale a UI value without rounding to whole numbers.
--
-- ui.rescaleFraction(val, baseResolution, rescaleToScreenAspect, targetResolution)
ui.rescaleFraction = function (val, baseResolution, rescaleToScreenAspect, targetResolution)
	return ui.rescaleUI(val, baseResolution, rescaleToScreenAspect, targetResolution, true)
end

require 'pigui.libs.wrappers'

local defaultTheme = require 'pigui.themes.default'

local pi = 3.14159264
local pi_2 = pi / 2
local pi_4 = pi / 4
local two_pi = pi * 2
local standard_gravity = 9.80665
local one_over_sqrt_two = 1 / math.sqrt(2)

ui.oneOverSqrtTwo = one_over_sqrt_two
ui.standardGravity = standard_gravity
ui.theme = defaultTheme
ui.twoPi = two_pi
ui.pi_2 = pi_2
ui.pi_4 = pi_4
ui.pi = pi

ui.anchor = { left = 1, right = 2, center = 3, top = 4, bottom = 5, baseline = 6 }
ui.fullScreenWindowFlags = ui.WindowFlags { "NoTitleBar", "NoResize", "NoMove", "NoInputs", "NoSavedSettings", "NoFocusOnAppearing", "NoBringToFrontOnFocus", "NoBackground" }

-- make all the necessary preparations for displaying the full-screen UI, launch the drawing function
function ui.makeFullScreenHandler(window_name, window_fnc)
	return function()
		ui.setNextWindowPos(Vector2(0, 0), "Always")
		ui.setNextWindowSize(Vector2(ui.screenWidth, ui.screenHeight), "Always")
		ui.window(window_name, ui.fullScreenWindowFlags, window_fnc)
	end
end

function ui.circleSegments(radius)
	if radius < 5 then
		return 8
	elseif radius < 20 then
		return 16
	elseif radius < 50 then
		return 32
	elseif radius < 100 then
		return 64
	else
		return 128
	end
end

local sides = {
	top = 1,
	bottom = 2,
	left = 3,
	right = 4
}

ui.sides = sides

-- Function: ui.rectcut
--
-- Simple rect-cutting UI layout implementation to cut and return space from
-- the given rectangle. This function intentionally modifies the input vectors.
--
-- Parameters:
--
--   min  - Vector2, minimum bound of the rectangle. Will be modified in-place.
--   max  - Vector2, maximum bound of the rectangle. Will be modified in-place.
--   amt  - number, amount of space (in pixels) to reserve from the rectangle.
--   side - integer, indicates the side of the rectangle to cut
--
-- Returns:
--
--   pos  - Vector2, upper-left corner of the reserved space
--   size - Vector2, x and y size of the reserved space
function ui.rectcut(min, max, amt, side)
	local size = max - min ---@type Vector2
	local pos = Vector2(0, 0)

	if side == sides.top then
		pos(min.x, min.y)
		size.y = math.min(size.y, amt)
		min.y = min.y + size.y
	elseif side == sides.bottom then
		size.y = math.min(size.y, amt)
		max.y = max.y - size.y
		pos(min.x, max.y)
	elseif side == sides.left then
		pos(min.x, min.y)
		size.x = math.min(size.x, amt)
		min.x = min.x + size.x
	else
		size.x = math.min(size.x, amt)
		max.x = max.x - size.x
		pos(max.x, min.y)
	end

	return pos, size
end

local modules = {}

-- Function: ui.registerModule
--
-- Register a modular widget for display in a specific view mode
--
-- Example:
--   > ui.registerModule('game', function() ... end)
--
-- Parameters:
--   mode - string, the UI binding point you want to register this module to
--   fun - a function (or table) that is responsible for drawing your custom UI.
--         If `fun` is a table, it should have a `draw` function. The `enabled`
--         key in the table is reserved for use by the module system.
function ui.registerModule(mode, fun)
	if not modules[mode] then
		modules[mode] = {}
	end
	if type(fun) == 'function' then fun = { draw = fun } end
	fun.enabled = true

	if fun.id and modules[mode][fun.id] then
		local idx = modules[mode][fun.id]
		modules[mode][idx] = fun
	else
		table.insert(modules[mode], fun)
		if fun.id then modules[mode][fun.id] = #modules[mode] end
	end
end

function ui.getModules(mode)
	return modules[mode] or {}
end

function ui.registerHandler(name, fun)
	pigui.handlers[name] = fun
end

function ui.registerTheme(name, theme)
	assert(type(theme) == "table", "UI Themes must be table values!")
	pigui.handlers[name] = theme
end

ui.registerTheme('default', defaultTheme)

return ui
