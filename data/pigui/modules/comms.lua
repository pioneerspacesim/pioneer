-- Copyright © 2008-2024 Pioneer Developers. See AUTHORS.txt for details
-- Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

local Game = require 'Game'
local utils = require 'utils'
local Vector2 = _G.Vector2

local Lang = require 'Lang'
local lc = Lang.GetResource("core");
local lui = Lang.GetResource("ui-core");

local ui = require 'pigui'

local gameView = require 'pigui.views.game'

local colors = ui.theme.colors
local icons = ui.theme.icons
local commsLogRetainTime = 120 -- how long before messages are removed from the short display

local function showItem(item)
	local color = colors.reticuleCircle
	if item.priority == 1 then
		color = colors.alertYellow
	elseif item.priority == 2 then
		color = colors.alertRed
	end
	if item.sender and item.sender ~= "" then
		ui.textColored(color, item.sender .. ": " .. item.text)
	else
		ui.textColored(color, item.text)
	end
end

-- Iterate through the CommsLog and collapse duplicate messages.
-- This should probably be done when adding a message to the log...
local function iterCommsLog(expireTime)
	local formatLine = function(line, rep)
		return {
			sender = line.sender, priority = line.priority,
			text = line.text .. ((rep > 1) and (' x ' .. rep) or '')
		}
	end

	local iter = function(lines, start)
		local line = nil
		local rep = 0

		for i, v in utils.reverse(lines, start) do
			if not line then
				-- skip processing if the messages are too old
				if expireTime and v.time < expireTime then
					return nil
				end

				-- process the first line
				line = v
				rep = 1
			elseif line.text == v.text and line.sender == v.sender then
				-- accumulate copies of the current line
				rep = rep + 1
				line = v
			else
				-- return the accumulated lines and process this index next time
				return i, formatLine(line, rep)
			end
		end

		-- we've reached the end of the list, send the last line if present
		if line then
			return 0, formatLine(line, rep)
		end
	end

	return iter, Game.GetCommsLines(), nil
end

gameView.registerSidebarModule("comms", {
	side = "left",
	icon = icons.comms,
	tooltip = lui.TOGGLE_FULL_COMMS_WINDOW,
	title = lc.COMMS,
	exclusive = true,
	drawBody = function()
		ui.pushTextWrapPos(0.0)

		for _, v in iterCommsLog() do
			showItem(v)
		end

		ui.popTextWrapPos()
	end
})

local windowFlags = {"NoTitleBar", "NoResize", "NoFocusOnAppearing", "NoBringToFrontOnFocus", "NoSavedSettings", "NoScrollbar"}

gameView.registerHudModule("comms", {
	side = "left",
	showInHyperspace = true,
	priority = 0, -- Comms should be drawn at the top
	debugReload = function() package.reimport() end,
	draw = function(_, min, max)
		local pos, size = ui.rectcut(min, max, ui.screenHeight / 8, ui.sides.top)
		ui.setNextWindowPos(pos, "Always")
		ui.setNextWindowSize(size, "Always")

		ui.window("ShortCommsLog", windowFlags, function()
			ui.withFont(ui.fonts.pionillium.details, function()
				ui.pushTextWrapPos(0.0)
				for _, v in iterCommsLog(Game.time - commsLogRetainTime) do
					showItem(v)
				end
				ui.popTextWrapPos()
			end)
		end)
	end
})

return {}
