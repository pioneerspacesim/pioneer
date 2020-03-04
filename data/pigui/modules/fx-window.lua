-- Copyright © 2008-2020 Pioneer Developers. See AUTHORS.txt for details
-- Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

local Engine = require 'Engine'
local Game = require 'Game'
local utils = require 'utils'
local Event = require 'Event'

local Lang = require 'Lang'
local lc = Lang.GetResource("core");
local lui = Lang.GetResource("ui-core");

local ui = require 'pigui'

local player = nil
local colors = ui.theme.colors
local icons = ui.theme.icons

local mainButtonSize = Vector2(32,32) * (ui.screenHeight / 1200)
local mainButtonFramePadding = 3
local function mainMenuButton(icon, selected, tooltip, color)
	if color == nil then
		color = colors.white
	end
	return ui.coloredSelectedIconButton(icon, mainButtonSize, selected, mainButtonFramePadding, colors.buttonBlue, color, tooltip)
end

local currentView = "internal"

local next_cam_type = { ["internal"] = "external", ["external"] = "sidereal", ["sidereal"] = "internal", ["flyby"] = "internal" }
local cam_tooltip = { ["internal"] = lui.HUD_BUTTON_INTERNAL_VIEW, ["external"] = lui.HUD_BUTTON_EXTERNAL_VIEW, ["sidereal"] = lui.HUD_BUTTON_SIDEREAL_VIEW, ["flyby"] = lui.HUD_BUTTON_FLYBY_VIEW }
local function button_world(current_view)
	ui.sameLine()
	if current_view ~= "world" then
		if mainMenuButton(icons.view_internal, false, lui.HUD_BUTTON_SWITCH_TO_WORLD_VIEW) or (ui.noModifierHeld() and ui.isKeyReleased(ui.keys.f1)) then
			Game.SetView("world")
			ui.playBoinkNoise()
		end
	else
		local camtype = Game.GetWorldCamType()
		if mainMenuButton(icons["view_" .. camtype], true, cam_tooltip[camtype]) or (ui.noModifierHeld() and ui.isKeyReleased(ui.keys.f1)) then
			Game.SetWorldCamType(next_cam_type[camtype])
			ui.playBoinkNoise()
		end
		if (ui.altHeld() and ui.isKeyReleased(ui.keys.f1)) then
			Game.SetWorldCamType("flyby")
			ui.playBoinkNoise()
		end
	end
end

local current_map_view = "sector"
local function buttons_map(current_view)
	local onmap = current_view == "sector" or current_view == "system" or current_view == "system_info"

	ui.sameLine()
	local active = current_view == "sector"
	if mainMenuButton(icons.sector_map, active, active and lui.HUD_BUTTON_SWITCH_TO_WORLD_VIEW or lui.HUD_BUTTON_SWITCH_TO_SECTOR_MAP) or (onmap and ui.noModifierHeld() and ui.isKeyReleased(ui.keys.f5)) then
		if active then
			Game.SetView("world")
		else
			Game.SetView("sector")
			current_map_view = "sector"
		end
	end

	ui.sameLine()
	active = current_view == "system"
	if mainMenuButton(icons.system_map, active, active and lui.HUD_BUTTON_SWITCH_TO_WORLD_VIEW or lui.HUD_BUTTON_SWITCH_TO_SYSTEM_MAP) or (onmap and ui.noModifierHeld() and ui.isKeyReleased(ui.keys.f6)) then
		if active then
			Game.SetView("world")
		else
			Game.SetView("system")
			current_map_view = "system"
		end
	end

	ui.sameLine()
	active = current_view == "system_info"
	if mainMenuButton(icons.system_overview, active, active and lui.HUD_BUTTON_SWITCH_TO_WORLD_VIEW or lui.HUD_BUTTON_SWITCH_TO_SYSTEM_OVERVIEW) or (onmap and ui.noModifierHeld() and ui.isKeyReleased(ui.keys.f7)) then
		if active then
			ui.systemInfoViewNextPage()
		else
			Game.SetView("system_info")
			current_map_view = "system_info"
		end
	end
	if ui.noModifierHeld() and ui.isKeyReleased(ui.keys.f2) then
		if onmap then
			Game.SetView("world")
		else
			Game.SetView(current_map_view)
		end
	end
end

local function button_info(current_view)
	ui.sameLine()
	if (mainMenuButton(icons.personal_info, current_view == "info", lui.HUD_BUTTON_SHOW_PERSONAL_INFO) or (ui.noModifierHeld() and ui.isKeyReleased(ui.keys.f3))) then
		if current_view ~= "info" then
			Game.SetView("info")
		else
			Game.SetView("world")
		end
	end
end

local function button_comms(current_view)
	if player:IsDocked() then
		ui.sameLine()
		if mainMenuButton(icons.comms, current_view == "space_station", lui.HUD_BUTTON_SHOW_COMMS) or (ui.noModifierHeld() and ui.isKeyReleased(ui.keys.f4)) then
			if current_view == "space_station" then
				Game.SetView("world")
			else
				Game.SetView("space_station")
			end
		end
	end
end

local function displayFxWindow()
	if ui.optionsWindow.isOpen then return end
	player = Game.player
	local current_view = Game.CurrentView()
	local aux = Vector2((mainButtonSize.x + mainButtonFramePadding * 2) * 10, (mainButtonSize.y + mainButtonFramePadding * 2) * 1.5)
	ui.setNextWindowSize(aux, "Always")
	aux = Vector2(ui.screenWidth/2 - (mainButtonSize.x + 4 * mainButtonFramePadding) * 7.5/2, 0)
	ui.setNextWindowPos(aux , "Always")
	ui.window("Fx", {"NoTitleBar", "NoResize", "NoFocusOnAppearing", "NoBringToFrontOnFocus", "NoScrollbar"},
						function()
							button_world(current_view)

							button_info(current_view)

							button_comms(current_view)

							buttons_map(current_view)
	end)
end

ui.registerModule("game", displayFxWindow)

return {}
