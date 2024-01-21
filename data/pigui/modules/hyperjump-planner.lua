-- Copyright © 2008-2024 Pioneer Developers. See AUTHORS.txt for details
-- Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

local Game = require 'Game'
local Equipment = require 'Equipment'
local Commodities = require 'Commodities'

local Lang = require 'Lang'
local lc = Lang.GetResource("core")
local lui = Lang.GetResource("ui-core");

local ui = require 'pigui'
local mb = require 'pigui.libs.message-box'

local colors = ui.theme.colors
local icons = ui.theme.icons
local Vector2 = _G.Vector2

local sectorView

local hyperJumpPlanner = {} -- for export

-- hyperjump route stuff
local hyperjump_route = {}
local route_jumps = 0
local current_system
local current_path
local map_selected_path
local selected_jump
local current_fuel
local remove_first_if_current = true
local textIconSize = nil

local function textIcon(icon, tooltip)
	ui.icon(icon, textIconSize, colors.font, tooltip)
end

local function showInfo()
	if ui.collapsingHeader(lui.ROUTE_INFO,{"DefaultOpen"}) then
		local total_fuel = 0
		local total_duration = 0
		local total_distance = 0

		textIcon(icons.navtarget, lui.CURRENT_SYSTEM)
		ui.sameLine()
		-- we can only have the current path in normal space
		if current_path then
			local start = current_path
			-- Tally up totals for the entire jump plan
			for _,jump in pairs(hyperjump_route) do
				local _, distance, fuel, duration = Game.player:GetHyperspaceDetails(start, jump.path)

				total_fuel = total_fuel + fuel
				total_duration = total_duration + duration
				total_distance = total_distance + distance

				start = jump.path
			end

			if ui.selectable(ui.Format.SystemPath(current_path)) then
				sectorView:SwitchToPath(current_path)
			end
		else -- no current path => we are hyperjumping => no current system
			ui.text("---")
		end

		textIcon(icons.route_destination, lui.FINAL_TARGET)

		if route_jumps > 0 then
			local final_path = hyperjump_route[route_jumps].path
			ui.sameLine()
			if ui.selectable(ui.Format.SystemPath(final_path), false, {}) then
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

local function smallButton(icon, tooltip, callback)
	local button = ui.mainMenuButton(icon, tooltip, nil, ui.theme.styles.SmallButtonSize)
	if button then
		callback()
	end
	return button
end -- smallButton

local function buildJumpRouteList()
	hyperjump_route = {}

	local player = Game.player
	-- if we are not in the system, then we are in hyperspace, we start building the route from the jump target
	local start = Game.system and Game.system.path or player:GetHyperspaceDestination()

	local drive = table.unpack(player:GetEquip("engine")) or nil
	local fuel_type = drive and drive.fuel or Commodities.hydrogen

	local cur_fuel = player:GetComponent('CargoManager'):CountCommodity(fuel_type)
	local running_fuel = 0

	for jumpIndex, jump in pairs(sectorView:GetRoute()) do
		local jump_sys = jump:GetSystemBody()
		local _, distance, fuel, _ = player:GetHyperspaceDetails(start, jump)
		local color
		local remaining_fuel = cur_fuel - running_fuel - fuel
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
	elseif not Game.InHyperspace() then
		-- we will not reset the hyperjump target while we are in the hyperjump
		sectorView:ResetHyperspaceTarget()
		selected_jump = nil
	end
end

local function showJumpRoute()
	if ui.collapsingHeader(lui.ROUTE_JUMPS, {"DefaultOpen"}) then
		smallButton(icons.forward, lui.ADD_JUMP,
			function()
				sectorView:AddToRoute(map_selected_path)
				updateHyperspaceTarget()
				selected_jump = #hyperjump_route
			end)
		ui.sameLine()

		smallButton(icons.current_line, lui.REMOVE_JUMP,
			function()
				if selected_jump then
					sectorView:RemoveRouteItem(selected_jump)
				end
				updateHyperspaceTarget()
			end)
		ui.sameLine()

		smallButton(icons.current_periapsis, lui.MOVE_UP,
			function()
				if selected_jump then
					if sectorView:MoveRouteItemUp(selected_jump) then
						selected_jump = selected_jump - 1
					end
				end
				updateHyperspaceTarget()
			end)
		ui.sameLine()

		smallButton(icons.current_apoapsis, lui.MOVE_DOWN,
			function()
				if selected_jump then
					if sectorView:MoveRouteItemDown(selected_jump) then
						selected_jump = selected_jump + 1
					end
				end
				updateHyperspaceTarget()
			end)
		ui.sameLine()

		smallButton(icons.retrograde_thin, lui.CLEAR_ROUTE,
			function()
				sectorView:ClearRoute()
				updateHyperspaceTarget()
			end)
		ui.sameLine()

		smallButton(icons.hyperspace, lui.AUTO_ROUTE,
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

		smallButton(icons.search_lens, lui.CENTER_ON_SYSTEM,
			function()
				if selected_jump then
					sectorView:GetMap():GotoSystemPath(hyperjump_route[selected_jump].path)
				end
			end)

		ui.separator()

		local clicked
		ui.child("routelist", function()
			for jumpIndex, jump in pairs(hyperjump_route) do
				ui.withStyleColors({["Text"] = jump.color}, function()
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
	smallButton(icons.forward, lui.ADD_JUMP, function() end)
	ui.sameLine()
	smallButton(icons.forward, lui.ADD_JUMP, function() end)
	ui.sameLine()
	smallButton(icons.forward, lui.ADD_JUMP, function() end)
	ui.sameLine()
	smallButton(icons.forward, lui.ADD_JUMP, function() end)
	ui.sameLine()
	smallButton(icons.forward, lui.ADD_JUMP, function() end)
	ui.sameLine()
	smallButton(icons.forward, lui.ADD_JUMP, function() end)
	ui.sameLine()
	smallButton(icons.forward, lui.ADD_JUMP, function() end)
	ui.separator()
	--reserve 5 route items
	ui.text("1: Barnard's Star (5.95ly - 1t) SPACE")
	ui.text("Route item")
	ui.text("Route item")
	ui.text("Route item")
	ui.text("Route item")
	ui.separator()
end

function hyperJumpPlanner.display()
	if not textIconSize then
		textIconSize = ui.calcTextSize("H")
		textIconSize.x = textIconSize.y -- make square
	end
	local drive = table.unpack(Game.player:GetEquip("engine")) or nil
	local fuel_type = drive and drive.fuel or Commodities.hydrogen
	current_system = Game.system -- will be nil during the hyperjump
	current_path = Game.system and current_system.path -- will be nil during the hyperjump
	current_fuel = Game.player:GetComponent('CargoManager'):CountCommodity(fuel_type)
	map_selected_path = sectorView:GetSelectedSystemPath()
	route_jumps = sectorView:GetRouteSize()
	showHyperJumpPlannerWindow()
end -- hyperJumpPlanner.display

function hyperJumpPlanner.setSectorView(sv)
	sectorView = sv
end

function hyperJumpPlanner.onPlayerCargoChanged(comm, count)
	local drive = table.unpack(Game.player:GetEquip("engine")) or nil
	local fuel_type = drive and drive.fuel or Commodities.hydrogen

	if comm.name == fuel_type.name then
		buildJumpRouteList()
	end
end

function hyperJumpPlanner.onShipEquipmentChanged(ship, equipment)
	if ship:IsPlayer() and equipment and equipment:IsValidSlot("engine", ship) then
		buildJumpRouteList()
	end
end

function hyperJumpPlanner.onEnterSystem(ship)
	-- remove the first jump if it's the current system (and enabled to do so)
	-- this should be the case if you are following a route and want the route to be
	-- updated as you make multiple jumps
	if ship:IsPlayer() and remove_first_if_current then
		if route_jumps > 0 and hyperjump_route[1] and hyperjump_route[1].path:IsSameSystem(Game.system.path) then
			sectorView:RemoveRouteItem(1)
		end
	end
	updateHyperspaceTarget()
end

function hyperJumpPlanner.onGameStart()
	-- get notified when the player buys hydrogen
	Game.player:GetComponent('CargoManager'):AddListener('hyperjump-planner', hyperJumpPlanner.onPlayerCargoChanged)
	-- we may have just loaded a jump route list, so lets build it fresh now
	buildJumpRouteList()

end

function hyperJumpPlanner.onGameEnd()
	-- clear the route out so it doesn't show up if the user starts a new game
	sectorView:ClearRoute()
	-- also clear the route list, saved in this module
	buildJumpRouteList()
end

return hyperJumpPlanner
