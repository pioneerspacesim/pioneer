-- Copyright © 2008-2020 Pioneer Developers. See AUTHORS.txt for details
-- Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

local Engine = require 'Engine'
local Input = require 'Input'
local Game = require 'Game'
local utils = require 'utils'
local Event = require 'Event'
local Vector2 = _G.Vector2

local Lang = require 'Lang'
local lc = Lang.GetResource("core");
local lui = Lang.GetResource("ui-core");

local ui = require 'pigui'

-- cache ui
local pionillium = ui.fonts.pionillium
local pionicons = ui.fonts.pionicons
local colors = ui.theme.colors
local icons = ui.theme.icons

local reticuleCircleRadius = math.min(ui.screenWidth, ui.screenHeight) / 8
local reticuleCircleThickness = 2.0

-- for modules
ui.reticuleCircleRadius = reticuleCircleRadius
ui.reticuleCircleThickness = reticuleCircleThickness

-- settings
local ASTEROID_RADIUS = 1500000 -- rocky planets smaller than this (in meters) are considered an asteroid, not a planet
local IN_SPACE_INDICATOR_SHIP_MAX_DISTANCE = 1000000 -- ships farther away than this don't show up on as in-space indicators
-- center of screen, set each frame by the handler
local center = nil

-- cache player each frame
local player = nil

-- cache some data each frame
local gameView = {
	center = nil,
	player = nil
}

import("pigui.libs.view-util").mixin_modules(gameView)

local function getBodyIcon(body)
	local st = body.superType
	local t = body.type
	if st == "STARPORT" then
		if t == "STARPORT_ORBITAL" then
			return icons.spacestation
		elseif body.type == "STARPORT_SURFACE" then
			return icons.starport
		end
	elseif st == "GAS_GIANT" then
		return icons.gas_giant
	elseif st == "STAR" then
		return icons.sun
	elseif st == "ROCKY_PLANET" then
		if body:IsMoon() then
			return icons.moon
		else
			local sb = body:GetSystemBody()
			if sb.radius < ASTEROID_RADIUS then
				return icons.asteroid_hollow
			else
				return icons.rocky_planet
			end
		end
	elseif body:IsShip() then
		local shipClass = body:GetShipClass()
		if icons[shipClass] then
			return icons[shipClass]
		else
			print("data/pigui/game.lua: getBodyIcon unknown ship class " .. (shipClass and shipClass or "nil"))
			return icons.ship -- TODO: better icon
		end
	elseif body:IsHyperspaceCloud() then
		return icons.hyperspace -- TODO: better icon
	elseif body:IsMissile() then
		return icons.bullseye -- TODO: better icon
	elseif body:IsCargoContainer() then
		return icons.rocky_planet -- TODO: better icon
	else
		print("data/pigui/game.lua: getBodyIcon not sure how to process body, supertype: " .. (st and st or "nil") .. ", type: " .. (t and t or "nil"))
		utils.print_r(body)
		return icons.ship
	end
end

local function setTarget(body)
	if body:IsShip() or body:IsMissile() then
		player:SetCombatTarget(body)
	else
		player:SetNavTarget(body)
	end
end

local function callModules(mode)
	for k,v in pairs(ui.getModules(mode)) do
		v.fun()
	end
end

local radial_menu_actions_orbital = {
	{icon = ui.theme.icons.normal_thin,
	 tooltip=lc.HEADING_LOCK_NORMAL,
	 action=function(target) Game.player:SetFlightControlState("CONTROL_FIXHEADING_NORMAL") end},
	{icon = ui.theme.icons.radial_out_thin,
	 tooltip=lc.HEADING_LOCK_RADIALLY_OUTWARD,
	 action=function(target) Game.player:SetFlightControlState("CONTROL_FIXHEADING_RADIALLY_OUTWARD") end},
	{icon = ui.theme.icons.retrograde_thin,
	 tooltip=lc.HEADING_LOCK_BACKWARD,
	 action=function(target) Game.player:SetFlightControlState("CONTROL_FIXHEADING_BACKWARD") end},
	{icon = ui.theme.icons.antinormal_thin,
	 tooltip=lc.HEADING_LOCK_ANTINORMAL,
	 action=function(target) Game.player:SetFlightControlState("CONTROL_FIXHEADING_ANTINORMAL") end},
	{icon = ui.theme.icons.radial_in_thin,
	 tooltip=lc.HEADING_LOCK_RADIALLY_INWARD,
	 action=function(target) Game.player:SetFlightControlState("CONTROL_FIXHEADING_RADIALLY_INWARD") end},
	{icon = ui.theme.icons.prograde_thin,
	 tooltip=lc.HEADING_LOCK_FORWARD,
	 action=function(target) Game.player:SetFlightControlState("CONTROL_FIXHEADING_FORWARD") end},
}

local function displayOnScreenObjects()
	if ui.altHeld() and not ui.isAnyWindowHovered() and ui.isMouseClicked(1) then
		local frame = player.frameBody
		if frame then
			ui.openRadialMenu(frame, 1, 30, radial_menu_actions_orbital)
		end
	end
	local navTarget = player:GetNavTarget()
	local combatTarget = player:GetCombatTarget()

	ui.radialMenu("onscreenobjects")

	local should_show_label = ui.shouldShowLabels()
	local iconsize = Vector2(18 , 18)
	local label_offset = 14 -- enough so that the target rectangle fits
	local cluster_size = iconsize.x -- size of clusters to be collapsed into single bodies
	local click_radius = cluster_size * 0.5
	-- make click_radius sufficiently smaller than the cluster size
	-- to prevent overlap of selection regions

	local bodies_grouped = ui.getProjectedBodiesGrouped(cluster_size, IN_SPACE_INDICATOR_SHIP_MAX_DISTANCE)

	for _,group in ipairs(bodies_grouped) do
		local mainBody = group.mainBody
		local mainCoords = group.screenCoordinates

		ui.addIcon(mainCoords, getBodyIcon(mainBody), colors.frame, iconsize, ui.anchor.center, ui.anchor.center)

		if should_show_label then
			local label = mainBody:GetLabel()
			if group.multiple then
				label = label .. " (" .. #group.bodies .. ")"
			end
			ui.addStyledText(mainCoords + Vector2(label_offset,0), ui.anchor.left, ui.anchor.center, label , colors.frame, pionillium.small)
		end
		local mp = ui.getMousePos()
		-- mouse release handler for radial menu
		if (mp - mainCoords):length() < click_radius then
			if not ui.isAnyWindowHovered() and ui.isMouseClicked(1) then
				local body = mainBody
				ui.openDefaultRadialMenu(body)
			end
		end
		-- mouse release handler
		if (mp - mainCoords):length() < click_radius then
			if not ui.isAnyWindowHovered() and ui.isMouseReleased(0) then
				if group.hasNavTarget or combatTarget == mainBody then
					-- if clicked and is target, unset target
					if group.hasNavTarget then
						player:SetNavTarget(nil)
					else
						player:SetCombatTarget(nil)
					end
					-- if not in setspeed mode or ctrl-click and is setspeed target,
					-- clear setspeed target
					if not player:GetSetSpeed() or (ui.ctrlHeld() and group.hasSetSpeedTarget) then
						player:SetSetSpeedTarget(nil)
					end
				elseif not group.multiple then
					-- clicked on single, just set navtarget/combatTarget
					setTarget(mainBody)
					if ui.ctrlHeld() then
						-- also set setspeed target on ctrl-click
						player:SetSetSpeedTarget(mainBody)
					elseif not player:GetSetSpeed() then
						-- clear setspeed target if not in setspeed mode
						player:SetSetSpeedTarget(nil)
					end
				else
					-- clicked on group, show popup
					ui.openPopup("navtarget" .. mainBody:GetLabel())
				end
			end
		end
		-- popup content
		ui.popup("navtarget" .. mainBody:GetLabel(), function()
			local small_iconsize = Vector2(16,16)
			for _,b in pairs(group.bodies) do
				ui.icon(getBodyIcon(b), small_iconsize, colors.frame)
				ui.sameLine()
				if ui.selectable(b:GetLabel(), b == navTarget, {}) then
					if b:IsShip() then
						player:SetCombatTarget(b)
					else
						player:SetNavTarget(b)
					end
					if ui.ctrlHeld() then
						-- also set setspeed target on ctrl-click
						player:SetSetSpeedTarget(b)
					elseif not player:GetSetSpeed() then
						-- clear setspeed target if not in setspeed mode
						player:SetSetSpeedTarget(nil)
					end
				end
			end
		end)
	end
end

gameView.registerModule("onscreen-objects", {
	showInHyperspace = false,
	draw = function(self, dT)
		displayOnScreenObjects()
	end
})

local function displayScreenshotInfo()
	if not Engine.GetDisableScreenshotInfo() then
		local current_system = Game.system
		if current_system then
			local current_path = current_system.path
			local frame = player.frameBody
			if frame then
				local info = frame.label .. ", " .. current_system.name .. " (" .. current_path.sectorX .. ", " .. current_path.sectorY .. ", " .. current_path.sectorZ .. ")"
				ui.addStyledText(Vector2(20, 20), ui.anchor.left, ui.anchor.top, info , colors.white, pionillium.large)
			end
		end
	end
end

local function drawGameModules()
	for i, module in ipairs(gameView.modules) do
		local shouldDraw = not Game.InHyperspace() or module.showInHyperspace
		if (not module.disabled) and shouldDraw then
			local ok, err = ui.pcall(module.draw, module, delta_t)
			if not ok then
				module.disabled = true
				print(err)
			end
		end
	end
end

local gameViewWindowFlags = ui.WindowFlags {"NoTitleBar", "NoResize", "NoMove", "NoInputs", "NoSavedSettings", "NoFocusOnAppearing", "NoBringToFrontOnFocus"}
ui.registerHandler('game', function(delta_t)
		-- delta_t is ignored for now
		player = Game.player
		gameView.player = player
		colors = ui.theme.colors -- if the theme changes
		icons = ui.theme.icons -- if the theme changes
		ui.setNextWindowPos(Vector2(0, 0), "Always")
		ui.setNextWindowSize(Vector2(ui.screenWidth, ui.screenHeight), "Always")
		ui.withStyleColors({ ["WindowBg"] = colors.transparent }, function()
			ui.window("HUD", gameViewWindowFlags, function()
				gameView.center = Vector2(ui.screenWidth / 2, ui.screenHeight / 2)
				if ui.shouldDrawUI() then
					if Game.CurrentView() == "world" then
						drawGameModules(gameView.modules)
						ui.radialMenu("worldloopworld")
					else
						ui.radialMenu("worldloopnotworld")
					end

					callModules("game")
				elseif Game.CurrentView() == "world" then
					displayScreenshotInfo()
				end
			end)
		end)

		if Game.CurrentView() == "world" and ui.noModifierHeld() and ui.isKeyReleased(ui.keys.escape) then
			if not ui.optionsWindow.isOpen then
				Game.SetTimeAcceleration("paused")
				ui.optionsWindow:open()
				Input.DisableBindings()
			else
				ui.optionsWindow:close()
				if not ui.optionsWindow.isOpen then
					Game.SetTimeAcceleration("1x")
					Input.EnableBindings()
				end
			end
		end

		callModules('modal')
end)

return gameView
