-- Copyright © 2008-2025 Pioneer Developers. See AUTHORS.txt for details
-- Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

local Game = require 'Game'
local Commodities = require 'Commodities'

local Lang = require 'Lang'
local lc = Lang.GetResource("core")
local lui = Lang.GetResource("ui-core");

local ui = require 'pigui'
local mb = require 'pigui.libs.message-box'

local colors = ui.theme.colors

local hyperJumpPlanner = {} -- for export
local sectorView

-- hyperjump route stuff
local hyperjump_route = {}
local selected_jump = 1
local remove_first_if_current = true

function hyperJumpPlanner.getRouteStats()
	local total_fuel = 0.0
	local total_duration = 0.0
	local total_distance = 0.0

	-- Tally up totals for the entire jump plan
	for _, jump in ipairs(hyperjump_route) do
		total_fuel = total_fuel + jump.fuel
		total_duration = total_duration + jump.duration
		total_distance = total_distance + jump.distance
	end

	return total_fuel, total_duration, total_distance
end

local function buildJumpRouteList()
	hyperjump_route = {}

	local player = Game.player
	-- if we are not in the system, then we are in hyperspace, we start building the route from the jump target
	local start = Game.system and Game.system.path or player:GetHyperspaceDestination()

	local drive = player:GetInstalledHyperdrive()
	if not drive then return end

	local cur_fuel = drive.storedFuel

	for jumpIndex, jump in pairs(sectorView:GetRoute()) do
		local _, distance, fuel, duration = player:GetHyperspaceDetails(start, jump)
		local color = colors.font
		local remaining_fuel = cur_fuel - fuel

		if remaining_fuel == 0 then
			color = colors.alertYellow
		elseif remaining_fuel < 0 then
			color = colors.alertRed
		end

		hyperjump_route[jumpIndex] = {
			path = jump,
			color = color,
			distance = distance,
			duration = duration,
			fuel = fuel,
			reachable = remaining_fuel >= 0
		}

		cur_fuel = cur_fuel - fuel
		start = jump
	end -- for
end

function hyperJumpPlanner.getJumpRouteList()
	return hyperjump_route
end

local function updateHyperspaceTarget()
	buildJumpRouteList()
	if #hyperjump_route > 0 then
		-- first waypoint is always the hyperspace target
		sectorView:SetHyperspaceTarget(hyperjump_route[1].path)
	elseif not Game.InHyperspace() then
		-- we will not reset the hyperjump target while we are in the hyperjump
		sectorView:ResetHyperspaceTarget()
		selected_jump = 1
	end
end

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
	selected_jump = 1
end

function hyperJumpPlanner.setSectorView(sv)
	sectorView = sv
end

function hyperJumpPlanner.addJump(path)
	sectorView:AddToRoute(path)
	updateHyperspaceTarget()
end

function hyperJumpPlanner.removeJump(index)
	sectorView:RemoveRouteItem(index)
	if selected_jump == index then
		selected_jump = math.max(1, index - 1)
	end
	updateHyperspaceTarget()
end

function hyperJumpPlanner.moveItemDown(index)
	if sectorView:MoveRouteItemDown(index) then
		if selected_jump == index then
			selected_jump = index + 1
		end
	end
	updateHyperspaceTarget()
end

function hyperJumpPlanner.moveItemUp(index)
	if sectorView:MoveRouteItemUp(index) then
		if selected_jump == index then
			selected_jump = index - 1
		end
	end
	updateHyperspaceTarget()
end

function hyperJumpPlanner.autoRoute(from, to)
	local result = sectorView:AutoRoute(from, to)
	if result == "NO_DRIVE" then
		mb.OK(lui.NO_DRIVE)
	elseif result == "NO_VALID_ROUTE" then
		mb.OK(lui.NO_VALID_ROUTE)
	end
	selected_jump = 1
	updateHyperspaceTarget()
end

function hyperJumpPlanner.setSelectedJump(index)
	selected_jump = index
end

function hyperJumpPlanner.getSelectedJump()
	return selected_jump
end

function hyperJumpPlanner.updateRouteList()
	updateHyperspaceTarget()
end

function hyperJumpPlanner.onPlayerCargoChanged(comm, count)
	local drive = Game.player:GetInstalledHyperdrive()
	local fuel_type = drive and drive.fuel or Commodities.hydrogen

	if comm.name == fuel_type.name then
		buildJumpRouteList()
	end
end

---@type EquipSet.Listener
function hyperJumpPlanner.onShipEquipmentChanged(op, equip, slot)
	if (op == 'install' or op == 'modify') and equip:IsA("Equipment.HyperdriveType") then
		buildJumpRouteList()
	end
end

function hyperJumpPlanner.onEnterSystem(ship)
	-- remove the first jump if it's the current system (and enabled to do so)
	-- this should be the case if you are following a route and want the route to be
	-- updated as you make multiple jumps
	if ship:IsPlayer() and remove_first_if_current then
		if #hyperjump_route > 0 and hyperjump_route[1] and hyperjump_route[1].path:IsSameSystem(Game.system.path) then
			sectorView:RemoveRouteItem(1)
		end
	end
	updateHyperspaceTarget()
end

function hyperJumpPlanner.onGameStart()
	-- get notified when the player buys hydrogen
	Game.player:GetComponent('CargoManager'):AddListener('hyperjump-planner', hyperJumpPlanner.onPlayerCargoChanged)
	Game.player:GetComponent('EquipSet'):AddListener(hyperJumpPlanner.onShipEquipmentChanged)
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
