-- Copyright © 2008-2022 Pioneer Developers. See AUTHORS.txt for details
-- Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

local Game = require 'Game'
local utils = require 'utils'
local Vector2 = _G.Vector2

local Lang = require 'Lang'
local lc = Lang.GetResource("core");
local lui = Lang.GetResource("ui-core");

local ui = require 'pigui'

local colors = ui.theme.colors
local icons = ui.theme.icons
local commsLogRetainTime = 60 -- how long messages are shown
local commsLinesToShow = 5 -- how many messages are shown

local fullComms

-- local lastLength = 0 -- how long the log was last frame
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

local function displayCommsLog()
	local aux = ui.getWindowPadding()
	local current_view = Game.CurrentView()
	if current_view == "world" then
		ui.setNextWindowPos(aux , "Always")
		ui.window("CommsLogButton", {"NoTitleBar", "NoResize", "NoFocusOnAppearing", "NoBringToFrontOnFocus", "NoSavedSettings"},
							function()
								if ui.mainMenuButton(icons.comms, lui.TOGGLE_FULL_COMMS_WINDOW) then
									fullComms = not fullComms
								end
		aux.x = aux.x + ui.getWindowSize().x
		end)
		ui.withFont(ui.fonts.pionillium.medium.name, ui.fonts.pionillium.medium.size, function()
									if not fullComms then -- not fullComms, show small window
										local size = Vector2(ui.screenWidth / 4, ui.screenHeight / 8)
										ui.setNextWindowSize(size , "Always")
										ui.setNextWindowPos(aux , "Always")
										ui.window("ShortCommsLog", {"NoTitleBar", "NoResize", "NoFocusOnAppearing", "NoBringToFrontOnFocus", "NoScrollbar"},
															function()
																local last = nil
																local rep = 0
																local commsLines = Game.GetCommsLines()
																local lines = {}
																for _,v in pairs(commsLines) do
																	if last and last.text == v.text and last.sender == v.sender then
																		rep = rep + 1
																		last = v
																	else
																		if rep > 0 then
																			table.insert(lines, 1, { sender = last.sender, text = last.text .. ((rep > 1) and (' x ' .. rep) or ''), priority = last.priority })
																			rep = 1
																			last = nil
																		end
																		if v.time > Game.time - commsLogRetainTime then
																			last = v
																			rep = 1
																		end
																	end
																end
																if last and last.time > Game.time - commsLogRetainTime then
																	table.insert(lines, 1, { sender = last.sender, text = last.text .. ((rep > 1) and (' x ' .. rep) or ''), priority = last.priority })
																end
																ui.pushTextWrapPos(ui.screenWidth/4 - 20)
																for _,v in pairs(utils.take(lines, commsLinesToShow)) do
																	showItem(v)
																end
																ui.popTextWrapPos()
										end)
									else  -- fullComms, show large window
										local size = Vector2(ui.screenWidth / 3, ui.screenHeight / 4)
										ui.setNextWindowSize(size , "Always")
										ui.setNextWindowPos(aux + Vector2(0, ui.getWindowPadding().y), "Always")
										ui.withStyleColors({ ["WindowBg"] = colors.commsWindowBackground }, function()
												ui.withStyleVars({ ["WindowRounding"] = 0.0 }, function()
														ui.window(lc.COMMS, {"NoResize"},
																			function()
																				local lines = Game.GetCommsLines()
																				ui.pushTextWrapPos(ui.screenWidth/3 - 20)
																				for _,v in pairs(utils.reverse(lines)) do
																					showItem(v)
																				end
																				ui.popTextWrapPos()
														end)
												end)
										end)
									end
		end) -- withFont
	end -- current_view == "world"
end

ui.registerModule("game", { id = 'comms', draw = displayCommsLog })

return {}
