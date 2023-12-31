-- Copyright © 2008-2024 Pioneer Developers. See AUTHORS.txt for details
-- Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

local Engine = require 'Engine'
local utils = require 'utils'

local Lang = require 'Lang'
local lc = Lang.GetResource("core");
local lui = Lang.GetResource("ui-core");

local ui = require 'pigui'

local base = Color(0,1,33)
local highlight = Color(0,63,112)
local transparent = Color(0,0,0,0)

local logo = ui.loadTextureFromSVG(ui.dataDirPath({"icons", "logo.svg"}), 512, 512)

-- support minimum resolution 800x600
local window_width = 700

ui.registerHandler(
	'init',
	function(progress)
		ui.setNextWindowPos(Vector2(ui.screenWidth / 2 - window_width / 2, ui.screenHeight/3*2), "Always")
		ui.setNextWindowSize(Vector2(window_width,200), "Always")
		ui.withFont("orbiteer", 18, function()
									ui.withStyleColors( {["WindowBg"] = transparent }, function ()
											ui.window("test", {"NoTitleBar", "NoResize", "NoMove"}, function()
																	local age = string.format("%.1f", 13.7 * progress)
																	local agestring = string.interp(lui.SIMULATING_UNIVERSE_EVOLUTION_N_BYEARS, { age = age })
																	local agesize = ui.calcTextSize(agestring)
																	ui.dummy(Vector2((window_width - agesize.x) / 2, 0))
																	ui.sameLine()
																	ui.text(agestring)

																	ui.dummy(Vector2(15,15))

																	ui.withStyleColors({ ["PlotHistogram"] = highlight, ["FrameBg"] = base }, function()
																			ui.progressBar(progress, Vector2(window_width - 20, ui.screenHeight / 43), "") -- 1080 / 43 -> 25
																	end)
											end)
									end)
		end)
		local logosize = (ui.screenHeight / 2.5)
		local leftup = Vector2(ui.screenWidth/2 - logosize/2, ui.screenHeight/7)
		ui.setNextWindowPos(leftup, "Always")
		ui.setNextWindowSize(Vector2(logosize, logosize), "Always")
		ui.withStyleColors( {["WindowBg"] = transparent }, function ()
				ui.window("logo", {"NoTitleBar", "NoResize", "NoMove"}, function ()
										local size = Vector2(logosize,logosize)
										local pos = Vector2(0,0) -- Vector2(ui.screenWidth / 2, ui.screenHeight / 3 * 2)
										local offset = ui.getWindowPos()
										ui.addImage(logo, pos + offset, pos + size + offset, Vector2(0.0, 0.0), Vector2(1.0, 1.0), Color(255,255,255))
				end)
		end)
end)
