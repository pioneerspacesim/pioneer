-- Copyright © 2008-2026 Pioneer Developers. See AUTHORS.txt for details
-- Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

local Game = require 'Game'
local Lang = require 'Lang'
local Vector2 = _G.Vector2

local ui = require 'pigui'

local lui = Lang.GetResource("ui-core")

local gameView = require 'pigui.views.game'

local pionillium = ui.fonts.pionillium
local colors = ui.theme.colors
local icons = ui.theme.icons

local flags = ui.WindowFlags { "NoDecoration", "NoFocusOnAppearing", "NoBringToFrontOnFocus" }

local function displayPlanetaryInfo()
	local player = Game.player

	if Game.CurrentView() ~= "WorldView" then
		return
	end

	local alt, vspd, latitude, longitude = player:GetGPS()
	if not alt then return end

	ui.withFont(pionillium.body, function()

		local width = gameView.rightSidebar.displaySize.x
		local height = ui.getTextLineHeightWithSpacing() * 3 - ui.getItemSpacing().y


		ui.setNextWindowSize(Vector2(width, height + ui.getWindowPadding().y * 2), "Always")
		ui.setNextWindowPos(ui.screenSize(), "Always", Vector2(1, 1))

		ui.window("PlanetaryInfo", flags, function()
			local label = Game.player.frameLabel

			if label then
				local sz = ui.calcTextSize(label)

				ui.addCursorPos(Vector2((ui.getContentRegion().x - sz.x) * 0.5, 0))
				ui.withStyleColors({ Text = ui.theme.styleColors.gray_300 }, function()
					ui.textShadowed(label)
				end)
			else
				ui.newLine()
			end

			ui.columns(2, "", false)

			local fmt = "%s %s"

			local orbit = player:GetOrbit()

			-- NOTE: these use em-dashes
			local ap = player.frameRotating and "—" or ui.Format.Distance(orbit.apogee)
			local pe = player.frameRotating and "—" or ui.Format.Distance(orbit.perigee)

			ui.textShadowed(fmt:format(ui.get_icon_glyph(icons.apoapsis), ap))
			ui.setItemTooltip(lui.HUD_ORBIT_APOAPSIS)

			ui.textShadowed(fmt:format(ui.get_icon_glyph(icons.periapsis), pe))
			ui.setItemTooltip(lui.HUD_ORBIT_PERIAPSIS)

			ui.nextColumn()

			ui.textShadowed(fmt:format(ui.get_icon_glyph(icons.latitude), ui.Format.Latitude(latitude)))
			ui.textShadowed(fmt:format(ui.get_icon_glyph(icons.longitude), ui.Format.Longitude(longitude)))

		end)

	end)
end

ui.registerModule("game", { id = "planetary-info", draw = displayPlanetaryInfo, debugReload = true })

return {}
