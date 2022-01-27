-- Copyright © 2008-2022 Pioneer Developers. See AUTHORS.txt for details
-- Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

local Engine = require 'Engine'
local pigui = Engine.pigui

local ui = require 'pigui.libs.forwarded'
ui.rescaleUI = require 'pigui.libs.rescale-ui'

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
		ui.window(window_name, ui.fullScreenWindowFlags, function()
			if ui.shouldDrawUI() then
				window_fnc()
			end
		end)
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
