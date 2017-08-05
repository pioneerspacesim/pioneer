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

local player = nil
local pionillium = ui.fonts.pionillium
local pionicons = ui.fonts.pionicons
local colors = ui.theme.colors
local icons = ui.theme.icons

local iconSize = Vector(16,16)

local function displayPlanetaryInfo()
	local player = Game.player
	local alt, vspd, latitude, longitude = player:GetGPS()
	if latitude and longitude and alt and vspd then
		ui.setNextWindowSize(Vector(140,120), "Always")
		ui.setNextWindowPos(Vector(ui.screenWidth - 150, ui.screenHeight - 100), "Always")
		ui.window("PlanetaryInfo", {"NoTitleBar", "NoResize", "NoFocusOnAppearing", "NoBringToFrontOnFocus"},
							function()
								ui.withFont(pionillium.medium.name, pionillium.medium.size, function()
															ui.icon(icons.altitude, iconSize, colors.reticuleCircle)
															ui.sameLine()
															local altitude,altitude_unit = ui.Format.Distance(alt)
															ui.text(altitude .. altitude_unit)
															ui.icon(icons.normal, iconSize, colors.reticuleCircle)
															ui.sameLine()
															local speed,speed_unit = ui.Format.Speed(vspd)
															ui.text(speed .. speed_unit)
															ui.icon(icons.latitude, iconSize, colors.reticuleCircle)
															ui.sameLine()
															ui.text(ui.Format.Latitude(latitude))
															ui.icon(icons.longitude, iconSize, colors.reticuleCircle)
															ui.sameLine()
															ui.text(ui.Format.Longitude(longitude))
								end)
		end)
	end
end

ui.registerModule("game", displayPlanetaryInfo)

return {}
