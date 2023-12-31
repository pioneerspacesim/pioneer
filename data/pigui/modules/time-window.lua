-- Copyright © 2008-2024 Pioneer Developers. See AUTHORS.txt for details
-- Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

local Game = require 'Game'
local Lang = require 'Lang'
local lc = Lang.GetResource("core");
local lui = Lang.GetResource("ui-core");
local ui = require 'pigui'
local icons = ui.theme.icons
local Vector2 = _G.Vector2
local timefont = ui.fonts.pionillium.large
local button_size = ui.theme.styles.MainButtonSize
local frame_padding = ui.theme.styles.MainButtonPadding
-- names of the keys in lang/core/
local months = {"MONTH_JAN", "MONTH_FEB", "MONTH_MAR", "MONTH_APR", "MONTH_MAY", "MONTH_JUN", "MONTH_JUL", "MONTH_AUG", "MONTH_SEP", "MONTH_OCT", "MONTH_NOV", "MONTH_DEC"}

local window_height = timefont.size + button_size.y + frame_padding * 2 + ui.getItemSpacing().y + ui.getWindowPadding().y * 2

local function displayTimeWindow()
	-- HACK: Don't display the time window if we're in a bespoke view
	if Game.CurrentView() == nil then return end

	local year, month_num, day, hour, minute, second = Game.GetDateTime()
	local month = lc[months[month_num]]
	local date = string.format("%04i %s %i - %02i:%02i:%02i", year, month, day, hour, minute, second)
	local current = Game.GetTimeAcceleration()
	local requested = Game.GetRequestedTimeAcceleration()

	local function accelButton(name, key)
		local state
		if requested == name and current ~= name then
			state = ui.theme.buttonColors.disabled
		else
			state = current == name -- true - 'selected' colorset, false - don't push buttoncolors
		end
		-- translate only paused, the rest can stay
		local time = (name == "paused") and lc.PAUSED or name
		local tooltip = string.interp(lui.HUD_REQUEST_TIME_ACCEL, { time = time })
		if ui.mainMenuButton(icons['time_accel_' .. name], tooltip, state)
			or (ui.shiftHeld() and ui.isKeyReleased(key)) then
			Game.SetTimeAcceleration(name, ui.ctrlHeld() or ui.isMouseDown(1))
		end
		-- isItemHovered is true for ALL the buttons
		if ui.isItemHovered(0) and ui.isMouseDoubleClicked(0) and (name == "paused") then
			ui.optionsWindow:open()
		end
		ui.sameLine()
	end

	local text_size
	ui.withFont(timefont.name, timefont.size, function()
		text_size = ui.calcTextSize(date)
	end)
	local buttons_width = (button_size.x + frame_padding * 2) * 6 + ui.getItemSpacing().x * 5
	local window_width = math.max(text_size.x, buttons_width) + ui.getWindowPadding().x * 2
	window_height = timefont.size + button_size.y + frame_padding * 2 + ui.getItemSpacing().y + ui.getWindowPadding().y * 2
	local window_size = Vector2(window_width, window_height)
	ui.timeWindowSize = window_size
	ui.setNextWindowSize(window_size, "Always")
	local window_pos = Vector2(0, ui.screenHeight - window_size.y)
	ui.setNextWindowPos(window_pos, "Always")
	ui.window("Time", {"NoTitleBar", "NoResize", "NoSavedSettings", "NoFocusOnAppearing", "NoBringToFrontOnFocus", "NoScrollbar"}, function()
		ui.withFont(timefont.name, timefont.size, function()
			ui.text(date)
		end)
		accelButton("paused", ui.keys.escape)
		accelButton("1x", ui.keys.f1)
		accelButton("10x", ui.keys.f2)
		accelButton("100x", ui.keys.f3)
		accelButton("1000x", ui.keys.f4)
		accelButton("10000x", ui.keys.f5)
	end)
end

ui.registerModule("game", { id = "time-window", draw = displayTimeWindow })

return { window_height = window_height }
