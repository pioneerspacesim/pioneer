-- Copyright © 2008-2017 Pioneer Developers. See AUTHORS.txt for details
-- Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

local Engine = import('Engine')
local ui = import('pigui')
local Vector = import('Vector')
local Color = import('Color')
local Lang = import("Lang")
local lc = Lang.GetResource("core");
local lui = Lang.GetResource("ui-core");
local utils = import("utils")

local base = Color(0,1,33)
local highlight = Color(0,63,112)

local logo = ui.loadTextureFromSVG(ui.dataDirPath({"icons", "logo.svg"}), 512, 512)

ui.registerHandler(
	'init',
	function(progress)
		ui.setNextWindowPos(Vector(0, ui.screenHeight/3*2), "Always")
		ui.withFont("orbiteer", 18, function()
									ui.window("test", {"NoTitleBar", "NoResize", "NoMove"}, function()
															local age = string.format("%.1f", 13.7 * progress)
															local agestring = string.interp(lui.SIMULATING_UNIVERSE_EVOLUTION_N_BYEARS, { age = age })
															local size = ui.calcTextSize(agestring)

															ui.dummy(Vector((ui.screenWidth - size.x) / 2, 0))
															ui.sameLine()
															ui.text(agestring)

															ui.dummy(Vector(15,15))

															ui.dummy(Vector(ui.screenWidth/4,0))
															ui.sameLine()
															ui.withStyleColors({ ["PlotHistogram"] = highlight, ["FrameBg"] = base }, function()
																	ui.progressBar(progress, Vector(ui.screenWidth / 2, ui.screenHeight / 43), "") -- 1080 / 43 -> 25
															end)
									end)
		end)
		local logosize = (ui.screenHeight / 2.5)
		local leftup = Vector(ui.screenWidth/2 - logosize/2, ui.screenHeight/7)
		ui.setNextWindowPos(leftup, "Always")
		ui.setNextWindowSize(Vector(logosize, logosize), "Always")
		ui.window("logo", {"NoTitleBar", "NoResize", "NoMove"}, function ()
								local size = Vector(logosize,logosize)
								local pos = Vector(0,0) -- Vector(ui.screenWidth / 2, ui.screenHeight / 3 * 2)
								local offset = ui.getWindowPos()
								ui.addImage(logo, pos + offset, pos + size + offset, Vector(0.0, 0.0), Vector(1.0, 1.0), Color(255,255,255))
		end)
end)
