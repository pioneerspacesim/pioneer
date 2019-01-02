-- Copyright © 2008-2019 Pioneer Developers. See AUTHORS.txt for details
-- Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

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
local colors = ui.theme.colors
local icons = ui.theme.icons

local mainButtonSize = Vector(32,32) * (ui.screenHeight / 1200)
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
		end
	else
		local camtype = Game.GetWorldCamType()
		if mainMenuButton(icons["view_" .. camtype], true, cam_tooltip[camtype]) or (ui.noModifierHeld() and ui.isKeyReleased(ui.keys.f1)) then
			Game.SetWorldCamType(next_cam_type[camtype])
		end
		if (ui.altHeld() and ui.isKeyReleased(ui.keys.f1)) then
			Game.SetWorldCamType("flyby")
		end
	end
end

local current_map_view = "sector"
local function buttons_map(current_view)
	local onmap = current_view == "sector" or current_view == "system" or current_view == "system_info" or current_view == "galaxy"

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
	if onmap then
		ui.sameLine()
		active = current_view == "galaxy"
		if mainMenuButton(icons.galaxy_map, active, active and lui.HUD_BUTTON_SWITCH_TO_WORLD_VIEW or lui.HUD_BUTTON_SWITCH_TO_GALAXY_MAP) or (onmap and ui.noModifierHeld() and ui.isKeyReleased(ui.keys.f8)) then
			if active then
				Game.SetView("world")
			else
				Game.SetView("galaxy")
				current_map_view = "galaxy"
			end
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
	ui.sameLine()
	if mainMenuButton(icons.comms, current_view == "space_station", lui.HUD_BUTTON_SHOW_COMMS) or (ui.noModifierHeld() and ui.isKeyReleased(ui.keys.f4)) then
		if player:IsDocked() then
			if current_view == "space_station" then
				Game.SetView("world")
			else
				Game.SetView("space_station")
			end
		else
			if ui.toggleSystemTargets then
				ui.toggleSystemTargets()
			end
		end
	end
end

local function displayFxWindow()
	if ui.showOptionsWindow then return end
	player = Game.player
	local current_view = Game.CurrentView()
	ui.setNextWindowSize(Vector((mainButtonSize.x + mainButtonFramePadding * 2) * 10, (mainButtonSize.y + mainButtonFramePadding * 2) * 1.5), "Always")
	ui.setNextWindowPos(Vector(ui.screenWidth/2 - (mainButtonSize.x + 4 * mainButtonFramePadding) * 7.5/2, 0) , "Always")
	ui.window("Fx", {"NoTitleBar", "NoResize", "NoFocusOnAppearing", "NoBringToFrontOnFocus"},
						function()
							button_world(current_view)

							button_info(current_view)

							button_comms(current_view)

							buttons_map(current_view)
	end)
end

ui.registerModule("game", displayFxWindow)

return {}
