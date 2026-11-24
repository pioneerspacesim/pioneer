-- Copyright © 2008-2026 Pioneer Developers. See AUTHORS.txt for details
-- Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

local Game = require 'Game'
local Lang = require 'Lang'
local lc = Lang.GetResource("core");
local ui = require 'pigui'
local Vector2 = _G.Vector2
local timefont = ui.fonts.pionillium.large

local window_height = 0

local windowFlags = ui.WindowFlags {"NoDecoration", "NoMove", "NoSavedSettings", "NoFocusOnAppearing", "NoBringToFrontOnFocus", "NoBackground"}

local function displayTimeAccelWindow()
	-- HACK: Don't display the time accel window if we're in a bespoke view
	if Game.CurrentView() == nil or Game.CurrentView() == "WorldView" and not ui.shouldDrawUI() then return end

	local current = Game.GetTimeAcceleration()
	-- translate only paused, the rest can stay
	local time = (current == "paused") and lc.PAUSED or current
	local text = string.format("Time Accel: %s", time)

	local text_size
	ui.withFont(timefont.name, timefont.size, function()
		text_size = ui.calcTextSize(text)
	end)

	local window_width = text_size.x + ui.getWindowPadding().x * 2
	window_height = timefont.size + ui.getWindowPadding().y * 2
	local window_size = Vector2(window_width, window_height)
	-- position about 300 pixels above the time window (which sits at the bottom of the screen)
	local window_pos = Vector2(0, ui.screenHeight - 300 - window_height)
	ui.setNextWindowSize(window_size, "Always")
	ui.setNextWindowPos(window_pos, "Always")
	ui.window("TimeAccel", windowFlags, function()
		ui.withFont(timefont.name, timefont.size, function()
			ui.textShadowed(text)
		end)
	end)
end

ui.registerModule("game", { id = "time-accel", draw = displayTimeAccelWindow })

return { window_height = window_height }
