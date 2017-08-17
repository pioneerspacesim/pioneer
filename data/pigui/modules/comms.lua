local Engine = import('Engine')
local Game = import('Game')
local ui = import('pigui/pigui.lua')
local Vector = import('Vector')
local Color = import('Color')
local Lang = import("Lang")
local lc = Lang.GetResource("core");
local lui = Lang.GetResource("ui-core");
local utils = import("utils")
local Event = import("Event")

local colors = ui.theme.colors
local icons = ui.theme.icons
local commsLogRetainTime = 60 -- how long messages are shown
local commsLinesToShow = 5 -- how many messages are shown

local mainButtonSize = Vector(32,32) * (ui.screenHeight / 1200)
local mainButtonFramePadding = 3
local fullComms

-- local lastLength = 0 -- how long the log was last frame
local function showItem(item)
	local color = colors.reticuleCircle
	if item.priority == 1 then
		color = colors.alertYellow
	elseif item.priority == 2 then
		color = colors.alertRed
	end
	ui.textColored(color, item.text)
end

local function displayCommsLog()
	local current_view = Game.CurrentView()
	if current_view == "world" then
		ui.setNextWindowPos(Vector(10, 10) , "Always")
		ui.window("CommsLogButton", {"NoTitleBar", "NoResize", "NoFocusOnAppearing", "NoBringToFrontOnFocus"},
							function()
								if ui.coloredSelectedIconButton(icons.comms, mainButtonSize, nil, mainButtonFramePadding, colors.buttonBlue, colors.white, 'Toggle full comms window') then
									fullComms = not fullComms
								end
		end)
		ui.withFont(ui.fonts.pionillium.medium.name, ui.fonts.pionillium.medium.size, function()
									if not fullComms then -- not fullComms, show small window
										ui.setNextWindowSize(Vector(ui.screenWidth / 4, ui.screenHeight / 8) , "Always")
										ui.setNextWindowPos(Vector(mainButtonSize.x + 2 * mainButtonFramePadding + 15, 10) , "Always")
										ui.window("ShortCommsLog", {"NoTitleBar", "NoResize", "NoFocusOnAppearing", "NoBringToFrontOnFocus"},
															function()
																local last = nil
																local rep = 0
																local commsLines = Game.GetCommsLines()
																local lines = {}
																for k,v in pairs(commsLines) do
																	if last and last.text == v.text then
																		rep = rep + 1
																		last = v
																	else
																		if rep > 0 then
																			table.insert(lines, 1, { text = last.text .. ((rep > 1) and (' x ' .. rep) or ''), priority = last.priority })
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
																	table.insert(lines, 1, { text = last.text .. ((rep > 1) and (' x ' .. rep) or ''), priority = last.priority })
																end
																ui.pushTextWrapPos(ui.screenWidth/4 - 20)
																for k,v in pairs(utils.reverse(utils.take(lines, commsLinesToShow))) do
																	showItem(v)
																end
																ui.popTextWrapPos()
										end)
									else  -- fullComms, show large window
										ui.setNextWindowSize(Vector(ui.screenWidth / 3, ui.screenHeight / 4) , "Always")
										ui.setNextWindowPos(Vector(mainButtonSize.x + 2 * mainButtonFramePadding + 25, 20) , "Always")
										ui.withStyleColors({ ["WindowBg"] = colors.commsWindowBackground }, function()
												ui.withStyleVars({ ["WindowRounding"] = 0.0 }, function()
														ui.window("CommsLog", {"NoResize"},
																			function()
																				local lines = Game.GetCommsLines()
																				ui.pushTextWrapPos(ui.screenWidth/3 - 20)
																				for k,v in pairs(lines) do
																					showItem(v)
																				end
																				ui.popTextWrapPos()
																				-- if lastLength ~= #lines then
																				-- 	ui.setScrollHere()
																				-- end
																				-- lastLength = #lines
														end)
												end)
										end)
									end
		end) -- withFont
	end -- current_view == "world"
end

ui.registerModule("game", displayCommsLog)

return {}
