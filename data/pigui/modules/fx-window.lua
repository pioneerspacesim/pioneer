-- Copyright © 2008-2025 Pioneer Developers. See AUTHORS.txt for details
-- Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

local Game = require 'Game'
local Lang = require 'Lang'
local lui = Lang.GetResource("ui-core");
local ui = require 'pigui'
local Vector2 = _G.Vector2

local player = nil
local icons = ui.theme.icons

local mainButtonSize = ui.theme.styles.MainButtonSize

local next_cam_type = { internal = "external", external = "sidereal", sidereal = "internal", flyby = "internal" }
local cam_tooltip = { internal = lui.HUD_BUTTON_INTERNAL_VIEW, external = lui.HUD_BUTTON_EXTERNAL_VIEW, sidereal = lui.HUD_BUTTON_SIDEREAL_VIEW, flyby = lui.HUD_BUTTON_FLYBY_VIEW }
local function button_world(current_view)
	local camtype = Game.GetWorldCamType()
	local view_icon = camtype and "view_" .. camtype or "view_internal"
	if current_view ~= "WorldView" then
		if ui.mainMenuButton(icons[view_icon], lui.HUD_BUTTON_SWITCH_TO_WORLD_VIEW) or (ui.noModifierHeld() and ui.isKeyReleased(ui.keys.f1)) then
			Game.SetView("WorldView")
			ui.playBoinkNoise()
		end
	else
		if ui.mainMenuButton(icons[view_icon], cam_tooltip[camtype], true) or (ui.noModifierHeld() and ui.isKeyReleased(ui.keys.f1)) then
			Game.SetWorldCamType(next_cam_type[camtype])
			ui.playBoinkNoise()
		end
		if (ui.altHeld() and ui.isKeyReleased(ui.keys.f1)) then
			Game.SetWorldCamType("flyby")
			ui.playBoinkNoise()
		end
	end
end

local current_map_view = "SectorView"
local function buttons_map(current_view)
	local onmap = current_view == "SectorView" or current_view == "SystemView"

	ui.sameLine()
	local active = current_view == "SectorView"
	if ui.mainMenuButton(icons.sector_map, lui.HUD_BUTTON_SWITCH_TO_SECTOR_MAP, active) or (onmap and ui.noModifierHeld() and ui.isKeyReleased(ui.keys.f5)) then
		if not active then
			Game.SetView("SectorView")
			current_map_view = "SectorView"
		end
	end

	ui.sameLine()
	local isOrrery = Game.systemView:GetDisplayMode() == "Orrery"
	active = current_view == "SystemView" and isOrrery
	if ui.mainMenuButton(icons.system_map, lui.HUD_BUTTON_SWITCH_TO_SYSTEM_MAP, active) or (onmap and ui.noModifierHeld() and ui.isKeyReleased(ui.keys.f6)) then
		if not active then
			Game.systemView:SetDisplayMode('Orrery')
			Game.SetView("SystemView")
			current_map_view = "SystemView"
		end
	end

	ui.sameLine()
	active = current_view == "SystemView" and not isOrrery
	if ui.mainMenuButton(icons.system_overview, lui.HUD_BUTTON_SWITCH_TO_SYSTEM_OVERVIEW, active) or (onmap and ui.noModifierHeld() and ui.isKeyReleased(ui.keys.f7)) then
		if not active then
			Game.systemView:SetDisplayMode('Atlas')
			Game.SetView("SystemView")
			current_map_view = "SystemView"
		end
	end

	if ui.noModifierHeld() and ui.isKeyReleased(ui.keys.f2) then
		if onmap then
			Game.SetView("WorldView")
		else
			Game.SetView(current_map_view)
		end
	end
end

local function button_info(current_view)
	ui.sameLine()
	local active = current_view == "InfoView"
	if ui.mainMenuButton(icons.personal_info, lui.HUD_BUTTON_SHOW_PERSONAL_INFO, active) or (ui.noModifierHeld() and ui.isKeyReleased(ui.keys.f3)) then
		if not active then
			Game.SetView("InfoView")
		end
	end
end

local function button_comms(current_view)
	if player:IsDocked() then
		ui.sameLine()
		local active = current_view == "StationView"
		if ui.mainMenuButton(icons.comms, lui.HUD_BUTTON_SHOW_COMMS, active) or (ui.noModifierHeld() and ui.isKeyReleased(ui.keys.f4)) then
			if not active then
				Game.SetView("StationView")
			end
		end
	end
end

local windowFlags = ui.WindowFlags {"NoTitleBar", "NoResize", "NoFocusOnAppearing", "NoBringToFrontOnFocus", "NoScrollbar"}

local function displayFxWindow()
	if ui.optionsWindow.isOpen then return end
	player = Game.player
	local current_view = Game.CurrentView()
	local window_width = mainButtonSize.x * 6 + ui.getItemSpacing().x * 5 + ui.getWindowPadding().x * 2
	local window_height = mainButtonSize.y + ui.getWindowPadding().y * 2
	ui.setNextWindowSize(Vector2(window_width, window_height), "Always")
	local aux = Vector2(ui.screenWidth / 2 - window_width / 2, 0)
	ui.setNextWindowPos(aux , "Always")
	ui.window("Fx", windowFlags, function()
		button_world(current_view)

		button_info(current_view)

		button_comms(current_view)

		buttons_map(current_view)
	end)
end

ui.registerModule("game", { id = "fx-window", draw = displayFxWindow })

return {}
