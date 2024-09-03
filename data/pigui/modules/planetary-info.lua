-- Copyright © 2008-2024 Pioneer Developers. See AUTHORS.txt for details
-- Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

local Game = require 'Game'
local ui = require 'pigui'
local Vector2 = _G.Vector2

local pionillium = ui.fonts.pionillium
local colors = ui.theme.colors
local icons = ui.theme.icons

local iconSize = Vector2(16,16)

local font = pionillium.medium
local width = 120 + 120 * (ui.screenWidth / 1200)
local height = math.max(iconSize.y, font.size) * 2 + ui.getItemSpacing().y + ui.getWindowPadding().y * 2

local function displayPlanetaryInfo()
	local player = Game.player
	local current_view = Game.CurrentView()
	if current_view == "world" then
		local alt, vspd, latitude, longitude = player:GetGPS()
		if latitude and longitude and alt and vspd then
			ui.setNextWindowSize(Vector2(width, height), "Always")
			ui.setNextWindowPos(Vector2(ui.screenWidth - width, ui.screenHeight - height), "Always")
			ui.window("PlanetaryInfo", {"NoTitleBar", "NoResize", "NoFocusOnAppearing", "NoBringToFrontOnFocus"},
				function()
					ui.withFont(font.name, font.size, function()
						ui.columns(2, "", false)
						ui.icon(icons.altitude, iconSize, colors.reticuleCircle)
						ui.sameLine()
						ui.text(ui.Format.Distance(alt))
						ui.icon(icons.normal, iconSize, colors.reticuleCircle)
						ui.sameLine()
						ui.text(ui.Format.Speed(vspd))
						ui.nextColumn()
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
end

ui.registerModule("game", { id = "planetary-info", draw = displayPlanetaryInfo })

return {}
