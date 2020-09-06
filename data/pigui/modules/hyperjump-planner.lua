-- Copyright © 2008-2020 Pioneer Developers. See AUTHORS.txt for details
-- Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

local Game = require 'Game'
local Event = require 'Event'
local Equipment = require 'Equipment'

local Lang = require 'Lang'
local lc = Lang.GetResource("core")
local lui = Lang.GetResource("ui-core");

local ui = require 'pigui'
local mb = require 'pigui.libs.message-box'

local player = nil
local colors = ui.theme.colors
local icons = ui.theme.icons

local sectorView

local mainButtonSize = ui.rescaleUI(Vector2(24,24), Vector2(1600, 900))
local mainButtonFramePadding = 3

local hyperJumpPlanner = {} -- for export

-- hyperjump route stuff
local hyperjump_route = {}
local route_jumps = 0
local auto_route_min_jump = 2 -- minimum jump distance when auto routing
local current_system
local current_path
local map_selected_path
local selected_jump
local current_fuel
local remove_first_if_current = true
local hideHyperJumpPlaner = false
local textIconSize = nil

local function textIcon(icon, tooltip)
	ui.icon(icon, textIconSize, colors.font, tooltip)
end

local function showJumpData(start, target, status, distance, fuel, duration, short)
	local color = colors.font
	if short then
		ui.withStyleColors({["Text"] = color}, function()

			ui.text(target:GetStarSystem().name)
			ui.sameLine()
			ui.text("("..fuel .. lc.UNIT_TONNES..")")
		end)
	else
		ui.withStyleColors({["Text"] = color}, function()
			ui.text(start:GetStarSystem().name)
			ui.sameLine()
			ui.text("->")
			ui.sameLine()
			ui.text(target:GetStarSystem().name)
			ui.sameLine()
			ui.text(":")
			ui.sameLine()
			ui.text(string.format("%.2f", distance) .. lc.UNIT_LY)
			ui.sameLine()
			ui.text(fuel .. lc.UNIT_TONNES)
			ui.sameLine()
			ui.text(ui.Format.Duration(duration, 2))
		end)
	end
end -- showJumpData

local function showInfo()
	if ui.collapsingHeader(lui.ROUTE_INFO,{"DefaultOpen"}) then
		local total_fuel = 0
		local total_duration = 0
		local total_distance = 0

		textIcon(icons.display_navtarget, lui.CURRENT_SYSTEM)
		ui.sameLine()
		-- we can only have the current path in normal space
		if current_path then
		local start = current_path
		-- Tally up totals for the entire jump plan
		for _,jump in pairs(hyperjump_route) do
			local status, distance, fuel, duration = player:GetHyperspaceDetails(start, jump.path)

			total_fuel = total_fuel + fuel
			total_duration = total_duration + duration
			total_distance = total_distance + distance

			start = jump.path
		end

		if ui.selectable(current_system.name .. " (" .. current_path.sectorX .. ", " .. current_path.sectorY .. ", " .. current_path.sectorZ ..")") then
			sectorView:SwitchToPath(current_path)
		end
		else -- no current path => we are hyperjumping => no current system
			ui.text("---")
		end

		textIcon(icons.route_destination, lui.FINAL_TARGET)

		if route_jumps > 0 then
			local final_path = hyperjump_route[route_jumps].path
			local final_sys = final_path:GetStarSystem()
			ui.sameLine()
			if ui.selectable(final_sys.name .. " (" .. final_path.sectorX .. ", " .. final_path.sectorY .. ", " .. final_path.sectorZ .. ")", false, {}) then
				sectorView:SwitchToPath(final_path)
			end
		else
			ui.sameLine()
			ui.text(lui.ADD_JUMP)
		end

		textIcon(icons.fuel, lui.REQUIRED_FUEL)
		ui.sameLine()
		ui.text(total_fuel .. lc.UNIT_TONNES)
		ui.sameLine()
		ui.text("[")
		ui.sameLine()
		ui.withStyleVars({ItemSpacing = Vector2(0.0)}, function()
			textIcon(icons.hull, lui.CURRENT_FUEL)
			ui.sameLine()
			ui.text(" : " .. current_fuel .. lc.UNIT_TONNES)
		end)
		ui.sameLine()
		ui.text("]")

		textIcon(icons.eta, lui.TOTAL_DURATION)
		ui.sameLine()
		ui.text(ui.Format.Duration(total_duration, 2))
		ui.sameLine()
		textIcon(icons.route_dist, lui.TOTAL_DISTANCE)
		ui.sameLine()
		ui.text(string.format("%.2f", total_distance) .. lc.UNIT_LY)
	end
end -- showInfo

local function mainButton(icon, tooltip, callback)
	local button = ui.coloredSelectedIconButton(icon, mainButtonSize, false, mainButtonFramePadding, colors.buttonBlue, colors.buttonInk, tooltip)
	if button then
		callback()
	end
	return button
end --mainButton

local function buildJumpRouteList()
	hyperjump_route = {}
	local player = Game.player
	local start = Game.system.path
	local drive = table.unpack(player:GetEquip("engine")) or nil
	local fuel_type = drive and drive.fuel or Equipment.cargo.hydrogen
	local current_fuel = player:CountEquip(fuel_type,"cargo")
	local running_fuel = 0
	for jumpIndex, jump in pairs(sectorView:GetRoute()) do
		local jump_sys = jump:GetSystemBody()
		local status, distance, fuel, duration = player:GetHyperspaceDetails(start, jump)
		local color
		local remaining_fuel = current_fuel - running_fuel - fuel
		if remaining_fuel == 0 then
			color = colors.alertYellow
		else
			if remaining_fuel < 0 then
				color = colors.alertRed
			else
				color = colors.font
			end
		end
		hyperjump_route[jumpIndex] = {
			path = jump,
			color = color,
			textLine = jumpIndex ..": ".. jump_sys.name .. " (" .. string.format("%.2f", distance) .. lc.UNIT_LY .. " - " .. fuel .. lc.UNIT_TONNES..")"
		}
		running_fuel = fuel + running_fuel
		start = jump
	end -- for
end

local function updateHyperspaceTarget()
	buildJumpRouteList()
	if #hyperjump_route > 0 then
		-- first waypoint is always the hyperspace target
		sectorView:SetHyperspaceTarget(hyperjump_route[1].path)
	else
		sectorView:ResetHyperspaceTarget()
		selected_jump = nil
	end
end

local function showJumpRoute()
	if ui.collapsingHeader(lui.ROUTE_JUMPS, {"DefaultOpen"}) then
		mainButton(icons.forward, lui.ADD_JUMP,
			function()
				sectorView:AddToRoute(map_selected_path)
				updateHyperspaceTarget()
				selected_jump = #hyperjump_route
		end)
		ui.sameLine()

		mainButton(icons.current_line, lui.REMOVE_JUMP,
			function()
				local new_route = {}
				local new_count = 0
				if selected_jump then
					sectorView:RemoveRouteItem(selected_jump)
				end
				updateHyperspaceTarget()
		end)
		ui.sameLine()

		mainButton(icons.current_periapsis, lui.MOVE_UP,
			function()
				if selected_jump then
					if sectorView:MoveRouteItemUp(selected_jump) then
						selected_jump = selected_jump - 1
					end
				end
				updateHyperspaceTarget()
		end)
		ui.sameLine()

		mainButton(icons.current_apoapsis, lui.MOVE_DOWN,
			function()
				if selected_jump then
					if sectorView:MoveRouteItemDown(selected_jump) then
						selected_jump = selected_jump + 1
					end
				end
				updateHyperspaceTarget()
		end)
		ui.sameLine()

		mainButton(icons.retrograde_thin, lui.CLEAR_ROUTE,
			function()
				sectorView:ClearRoute()
				updateHyperspaceTarget()
		end)
		ui.sameLine()

		mainButton(icons.hyperspace, lui.AUTO_ROUTE,
			function()
				local result = sectorView:AutoRoute()
				if result == "NO_DRIVE" then
					mb.OK(lui.NO_DRIVE)
				elseif result == "NO_VALID_ROUTE" then
					mb.OK(lui.NO_VALID_ROUTE)
				end
				updateHyperspaceTarget()
		end)
		ui.sameLine()

		mainButton(icons.search_lens, lui.CENTER_ON_SYSTEM,
			function()
				if selected_jump then
					sectorView:GotoSystemPath(hyperjump_route[selected_jump].path)
				end
		end)

		ui.separator()

		local clicked
		ui.child("routelist", function()
		for jumpIndex, jump in pairs(hyperjump_route) do
			ui.withStyleColors({["Text"] = jump.color},
				function()
					if ui.selectable(jump.textLine, jumpIndex == selected_jump) then
						clicked = jumpIndex
					end
			end)
		end -- for
		end --function
		)

		if clicked then
			selected_jump = clicked
			sectorView:SwitchToPath(hyperjump_route[selected_jump].path)
		end
	end
end -- showJumpPlan

-- scan the route and if this system is there, but another star is selected, update it in route
function hyperJumpPlanner.updateInRoute(path)
	for jumpIndex, jump in pairs(hyperjump_route) do
		if jump.path:IsSameSystem(path) then
			selected_jump = jumpIndex
			if jump ~= path then
				sectorView:UpdateRouteItem(jumpIndex, path)
				updateHyperspaceTarget()
			end
			return
		end
	end
	selected_jump = nil;
end

local function showHyperJumpPlannerWindow()
				textIcon(icons.route)
				ui.sameLine()
				ui.text(lui.HYPERJUMP_ROUTE)
				ui.separator()
				showInfo()
				ui.separator()
				showJumpRoute()
end -- showHyperJumpPlannerWindow

function hyperJumpPlanner.Dummy()
	ui.text("Hyperjump route")
	ui.separator()
	ui.collapsingHeader("Route info",{"DefaultOpen"})
	ui.text("Current system")
	ui.text("Final target")
	ui.text("Fuel line")
	ui.text("Duration line")
	ui.collapsingHeader("Route jumps",{"DefaultOpen"})
	mainButton(icons.forward, lui.ADD_JUMP, function() end)
	ui.sameLine()
	mainButton(icons.forward, lui.ADD_JUMP, function() end)
	ui.sameLine()
	mainButton(icons.forward, lui.ADD_JUMP, function() end)
	ui.sameLine()
	mainButton(icons.forward, lui.ADD_JUMP, function() end)
	ui.sameLine()
	mainButton(icons.forward, lui.ADD_JUMP, function() end)
	ui.sameLine()
	mainButton(icons.forward, lui.ADD_JUMP, function() end)
	ui.sameLine()
	mainButton(icons.forward, lui.ADD_JUMP, function() end)
	ui.separator()
	--reserve 5 route items
	ui.text("Route item")
	ui.text("Route item")
	ui.text("Route item")
	ui.text("Route item")
	ui.text("Route item")
	ui.separator()
end

function hyperJumpPlanner.display()
	player = Game.player
	if not textIconSize then
			textIconSize = ui.calcTextSize("H")
			textIconSize.x = textIconSize.y -- make square
	end
		local drive = table.unpack(player:GetEquip("engine")) or nil
		local fuel_type = drive and drive.fuel or Equipment.cargo.hydrogen
		current_system = Game.system -- will be nil during the hyperjump
		current_path = Game.system and current_system.path -- will be nil during the hyperjump
		current_fuel = player:CountEquip(fuel_type,"cargo")
		map_selected_path = sectorView:GetSelectedSystemPath()
		route_jumps = sectorView:GetRouteSize()
		showHyperJumpPlannerWindow()
end -- hyperJumpPlanner.display

function hyperJumpPlanner.setSectorView(sv)
	sectorView = sv
end

function hyperJumpPlanner.onShipEquipmentChanged(ship, equipment)
	if ship:IsPlayer() and equipment and (equipment:GetName() == "Hydrogen"  or equipment:IsValidSlot("engine", ship)) then
		buildJumpRouteList()
	end
end

function hyperJumpPlanner.onEnterSystem(ship)
		-- remove the first jump if it's the current system (and enabled to do so)
		-- this should be the case if you are following a route and want the route to be
		-- updated as you make multiple jumps
		if ship:IsPlayer() and remove_first_if_current then
			if route_jumps > 0 and hyperjump_route[1].path:IsSameSystem(Game.system.path) then
				sectorView:RemoveRouteItem(1)
			end
		end
	updateHyperspaceTarget()
end

function hyperJumpPlanner.onGameEnd(ship)
	-- clear the route out so it doesn't show up if the user starts a new game
	sectorView:ClearRoute()
	-- also clear the route list, saved in this module
	buildJumpRouteList()
end

return hyperJumpPlanner
