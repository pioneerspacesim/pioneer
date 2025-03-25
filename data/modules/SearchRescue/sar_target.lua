-- Copyright © 2008-2025 Pioneer Developers. See AUTHORS.txt for details
-- Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

-- This module defines "Search and Rescue" functions that deal with target ship interactions.


local Engine = require 'Engine'
local Lang = require 'Lang'
local Game = require 'Game'
local Comms = require 'Comms'
local Commodities = require 'Commodities'
local ShipDef = require 'ShipDef'
local Timer = require 'Timer'
local l = Lang.GetResource("module-searchrescue")

local sar_utils = require 'modules.SearchRescue.sar_utils'
local sar_config = require 'modules.SearchRescue.sar_config'


local sar_target = {
   player_leaving_system = false,       -- needed for last minute cleanup before leaving system (searchForTarget)
}


function targetInteractionDistance (mission)
	-- Determine if player is within interaction distance from mission target.
	if mission.target and Game.player:DistanceTo(mission.target) <= sar_config.min_interaction_dist then
		return true
	else
		return false
	end
end

function pickupCrew (mission)
	-- Pickup a single crew member from the target ship.
	-- Called during timer loop within "interactWithTarget".
	local todo = mission.pickup_crew_orig

	-- error messages if not all parameters met
	if not sar_utils.crewPresent(mission.target) then
		Comms.ImportantMessage(l.MISSING_CREW)
		mission.pickup_crew_check = "PARTIAL"
		return
	elseif not sar_utils.passengerSpace(Game.player) then
		Comms.ImportantMessage(l.FULL_PASSENGERS)
		local done = mission.pickup_crew_orig - mission.pickup_crew
		local resulttxt = string.interp(l.RESULT_PICKUP_CREW, {todo = todo, done = done})
		Comms.ImportantMessage(resulttxt)
		if mission.pickup_pass > 0 then
			local todo_pass = mission.pickup_pass_orig
			local done_pass = mission.pickup_pass_orig - mission.pickup_pass
			local resulttxt_pass = string.interp(l.RESULT_PICKUP_PASS, {todo = todo_pass, done = done_pass})
			Comms.ImportantMessage(resulttxt_pass)
		end
		mission.pickup_crew_check = "PARTIAL"
		return

	-- pickup crew
	else
		local crew_member = sar_utils.removeCrew(mission.target)
		sar_utils.addPassenger(Game.player, crew_member)
		table.insert(mission.cargo_pass, crew_member)
		local boardedtxt = string.interp(l.BOARDED_PASSENGER, {name = crew_member.name})
		Comms.ImportantMessage(boardedtxt)
		mission.crew_num = mission.crew_num - 1

		-- if all necessary crew transferred print result message
		mission.pickup_crew = mission.pickup_crew - 1
		local done = mission.pickup_crew_orig - mission.pickup_crew
		if todo == done then
			local resulttxt = string.interp(l.RESULT_PICKUP_CREW, {todo = todo, done = done})
			Comms.ImportantMessage(resulttxt)
			mission.pickup_crew_check = "COMPLETE"
			mission.location = mission.system_local
		end
	end
end

local pickupPassenger = function (mission)
	-- Pickup a single passenger from the target ship.
	-- Called during timer loop within "interactWithTarget".
	local todo = mission.pickup_pass_orig

	-- error messages if not all parameters met
	if not sar_utils.passengersPresent(mission.target) then
		Comms.ImportantMessage(l.MISSING_PASS)
		mission.pickup_pass_check = "PARTIAL"
		return
	elseif not sar_utils.passengerSpace(Game.player) then
		Comms.ImportantMessage(l.FULL_PASSENGERS)
		local done = mission.pickup_pass_orig - mission.pickup_pass
		local resulttxt = string.interp(l.RESULT_PICKUP_PASS, {todo = todo, done = done})
		Comms.ImportantMessage(resulttxt)
		mission.pickup_pass_check = "PARTIAL"
		return

	-- pickup passenger
	else
		local passenger = table.remove(mission.return_pass)
		sar_utils.removePassenger(mission.target, passenger)
		sar_utils.addPassenger(Game.player, passenger)
		table.insert(mission.cargo_pass, passenger)
		local boardedtxt = string.interp(l.BOARDED_PASSENGER, {name = passenger.name})
		Comms.ImportantMessage(boardedtxt)

		-- if all necessary passengers have been picked up show result message
		mission.pickup_pass = mission.pickup_pass - 1
		local done = mission.pickup_pass_orig - mission.pickup_pass
		if todo == done then
			local resulttxt = string.interp(l.RESULT_PICKUP_PASS, {todo = todo, done = done})
			Comms.ImportantMessage(resulttxt)
			mission.pickup_pass_check = "COMPLETE"
		end
	end
end

local pickupCommodity = function (mission, commodity)
	-- Pickup a single ton of the supplied commodity from the target ship.
	-- Called during timer loop within "interactWithTarget".
	local todo = mission.pickup_comm_orig[commodity]
	local commodity_name = commodity:GetName()

	-- error messages if parameters not met
	if not sar_utils.cargoPresent(mission.target, commodity) then
		local missingtxt = string.interp(l.MISSING_COMM, {cargotype = commodity_name})
		Comms.ImportantMessage(missingtxt)
		mission.pickup_comm_check[commodity] = "PARTIAL"
		return
	elseif not sar_utils.cargoSpace(Game.player) then
		Comms.ImportantMessage(l.FULL_CARGO)
		mission.pickup_comm_check[commodity] = "PARTIAL"
		return

	-- pickup 1 ton of commodity
	else
		sar_utils.removeCargo(mission.target, commodity)
		sar_utils.addCargo(Game.player, commodity)
		if mission.cargo_comm[commodity] == nil then
			mission.cargo_comm[commodity] = 1
		else
			mission.cargo_comm[commodity] = mission.cargo_comm[commodity] + 1
		end

		-- show result message if done picking up this commodity
		mission.pickup_comm[commodity] = mission.pickup_comm[commodity] - 1
		local done = mission.pickup_comm_orig[commodity] - mission.pickup_comm[commodity]
		if todo == done then
			local resulttxt = string.interp(l.RESULT_PICKUP_COMM, {cargotype = commodity_name, todo = todo, done = done})
			Comms.ImportantMessage(resulttxt)
			mission.pickup_comm_check[commodity] = "COMPLETE"
		end
	end
end

local deliverCrew = function (mission)
	-- Transfer a single crew member to the target ship.
	-- Called during timer loop within "interactWithTarget".
	local todo = mission.deliver_crew_orig
	local maxcrew = ShipDef[mission.target.shipId].maxCrew

	-- error messages if not all parameters met
	if not sar_utils.passengersPresent(Game.player) then
		Comms.ImportantMessage(l.MISSING_PASSENGER)
		mission.deliver_crew_check = "PARTIAL"
		return
	elseif mission.target:CrewNumber() == maxcrew then
		Comms.ImportantMessage(l.FULL_CREW)
		mission.deliver_crew_check = "PARTIAL"
		return

	-- transfer crew
	else
		local crew_member = table.remove(mission.cargo_pass, 1)
		sar_utils.removePassenger(Game.player, crew_member)
		sar_utils.addCrew(mission.target, crew_member)
		mission.crew_num = mission.crew_num + 1
		local deliverytxt = string.interp(l.DELIVERED_PASSENGER, {name = crew_member.name})
		Comms.ImportantMessage(deliverytxt)

		-- if all necessary crew transferred print result message
		mission.deliver_crew = mission.deliver_crew - 1
		local done = mission.deliver_crew_orig - mission.deliver_crew
		if todo == done then
			local resulttxt = string.interp(l.RESULT_DELIVERY_CREW, {todo = todo, done = done})
			Comms.ImportantMessage(resulttxt)
			mission.deliver_crew_check = "COMPLETE"
		end
	end
end

local deliverPassenger = function (mission)
	-- Transfer a single passenger to the target ship.
	-- Called during timer loop within "interactWithTarget".
	local todo = mission.deliver_pass_orig

	-- error messages if not all parameters met
	if not sar_utils.passengersPresent(Game.player) then
		Comms.ImportantMessage(l.MISSING_PASS)
		mission.deliver_pass_check = "PARTIAL"
		return
	elseif not sar_utils.passengerSpace(mission.target) then
		Comms.ImportantMessage(l.FULL_PASSENGERS)
		mission.deliver_pass_check = "PARTIAL"
		return

	-- transfer passenger
	else
		local passenger = table.remove(mission.cargo_pass, 1)
		sar_utils.removePassenger(Game.player, passenger)
		sar_utils.addPassenger(mission.target, passenger)
		local deliverytxt = string.interp(l.DELIVERED_PASSENGER, {name = passenger.name})
		Comms.ImportantMessage(deliverytxt)

		-- if all necessary passengers have been transferred show result message
		mission.deliver_pass = mission.deliver_pass - 1
		local done = mission.deliver_pass_orig - mission.deliver_pass
		if todo == done then
			local resulttxt = string.interp(l.RESULT_DELIVERY_PASS, {todo = todo, done = done})
			Comms.ImportantMessage(resulttxt)
			mission.deliver_pass_check = "COMPLETE"
		end
	end
end

local deliverCommodity = function (mission, commodity)
	-- Transfer a single ton of the supplied commodity to the target ship.
	-- Called during timer loop within "interactWithTarget".
	local todo = mission.deliver_comm_orig[commodity]
	local commodity_name = commodity:GetName()

	-- error messages if parameters not met
	if not sar_utils.cargoPresent(Game.player, commodity) then
		local missingtxt = string.interp(l.MISSING_COMM, {cargotype = commodity_name})
		Comms.ImportantMessage(missingtxt)
		mission.deliver_comm_check[commodity] = "PARTIAL"
		return
	elseif not sar_utils.cargoSpace(mission.target) then
		Comms.ImportantMessage(l.FULL_CARGO)
		mission.deliver_comm_check[commodity] = "PARTIAL"
		return

	-- transfer 1 ton of commodity
	else
		sar_utils.removeCargo(Game.player, commodity)
		sar_utils.addCargo(mission.target, commodity)

		-- if commodity was fuel and the mission was local refuel the ship with it
		-- prevents issues where the ship's spare cargo space is smaller than the fuel we're delivering
		if commodity == Commodities.hydrogen then
			if mission.flavour.id == 2 or mission.flavour.id == 4 or mission.flavour.id == 5 then
				mission.target:Refuel(Commodities.hydrogen, 1)
			end
		end

		-- show result message if done delivering this commodity
		mission.deliver_comm[commodity] = mission.deliver_comm[commodity] - 1
		local done = mission.deliver_comm_orig[commodity] - mission.deliver_comm[commodity]
		if todo == done then
			local resulttxt = string.interp(l.RESULT_DELIVERY_COMM, {done = done, todo = todo, cargotype = commodity_name})
			Comms.ImportantMessage(resulttxt)
			mission.deliver_comm_check[commodity] = "COMPLETE"
		end
	end
end

local interactionCounter = function (counter)
	-- Check if target_interaction_time is reached.
	-- Called during timer loop inside "interactWithTarget".
	counter = counter + 1
	if counter >= sar_config.target_interaction_time then
		return true, counter
	else
		return false, counter
	end
end


local interactWithTarget = function (mission)
	-- Handle all interaction with mission target once the player ship is within interaction distance.
	if Game.time > mission.due then
		Comms.ImportantMessage(l.SHIP_UNRESPONSIVE)
		return
	else
		-- calculate and display total interaction time
		local packages
		packages = mission.pickup_crew + mission.pickup_pass +
			mission.deliver_crew + mission.deliver_pass
		for _,num in pairs(mission.pickup_comm) do
			packages = packages + num
		end
		for _,num in pairs(mission.deliver_comm) do
			packages = packages + num
		end
		local total_interaction_time = sar_config.target_interaction_time * packages
		local distance_reached_txt = string.interp(l.INTERACTION_DISTANCE_REACHED, {minutes = total_interaction_time/60})
		Comms.ImportantMessage(distance_reached_txt)
	end

	local counter = 0
	Timer:CallEvery(1, function ()
		local done = true

		-- abort if interaciton distance was not held or target ship destroyed
		-- TODO: set the check mark for each mission right
		if not targetInteractionDistance(mission) or mission.target == nil then
			Comms.ImportantMessage(l.INTERACTION_ABORTED)
			searchForTarget(mission)
			return true
		end

		-- perform action if time limit has passed
		local actiontime
		actiontime, counter = interactionCounter(counter)
		if actiontime then

			-- pickup crew from target ship
			if mission.pickup_crew > 0 then
				pickupCrew(mission)
				if mission.pickup_crew_check ~= "PARTIAL" then
					done = false
				end

				-- transfer crew to target ship
			elseif mission.deliver_crew > 0 then
				deliverCrew(mission)
				if mission.deliver_crew_check ~= "PARTIAL" then
					done = false
				end

				-- pickup passengers from target ship
			elseif mission.pickup_pass > 0 then
				pickupPassenger(mission)
				if mission.pickup_pass_check ~= "PARTIAL" then
					done = false
				end

				-- transfer passengers to target ship
			elseif mission.deliver_pass > 0 then
				deliverPassenger(mission)
				if mission.deliver_pass_check ~= "PARTIAL" then
					done = false
				end

				-- pickup commodity-cargo from target ship
			else
				for commodity, _ in pairs(mission.pickup_comm) do
					if mission.pickup_comm[commodity] > 0 then
						pickupCommodity(mission, commodity)
						if mission.pickup_comm_check[commodity] == "PARTIAL" then
							done = false
						end
					end
				end

				-- transfer commodity-cargo to target ship
				for commodity, _ in pairs(mission.deliver_comm) do
					if mission.deliver_comm[commodity] > 0 then
						deliverCommodity(mission, commodity)
						if mission.deliver_comm_check[commodity] ~= "PARTIAL" then
							done = false
						end
					end
				end
			end

			if done then

				-- if mission should close right after transfer do so and send target ship on its way
				if missionStatus(mission) == "COMPLETE" and mission.flavour.reward_immediate == true then
					closeMission(mission)

					-- wait for random time then fly off
					local wait_secs = Engine.rand:Integer(2,5)
					Timer:CallAt(Game.time + wait_secs, function () flyToNearbyStation(mission.target) end)
				end
				return true
			end
		end
	end)
end


local flyToNearbyStation =  function (ship)
	-- Fly the supplied ship to the closest station. If no station is in the system then jump to the
	-- closest system that does have a station.

	-- check if ship has atmo shield and limit to vacuum starports if not
	local vacuum = true
	if (ship["atmo_shield_cap"] or 0) > 1 then
		vacuum = false
	end

	local nearbysystems
	local nearbystations = findNearbyStations(vacuum, ship)

	if #nearbystations > 0 then

		-- add ship to discard pile to jump away later
		table.insert(discarded_ships, ship)

		-- blast off ship if LANDED
		if ship.flightState == "LANDED" then
			ship:AIEnterLowOrbit(ship:FindNearestTo("PLANET") or ship:FindNearestTo("STAR"))
			Timer:CallAt(Game.time + 5, function () ship:AIDockWith(Space.GetBody(nearbystations[1].bodyIndex)) end)
		else
			ship:AIDockWith(Space.GetBody(nearbystations[1].bodyIndex))
		end
	else
		local with_stations = true
		nearbysystems = findNearbySystems(with_stations)

		-- blast off ship if LANDED, otherwise hyp away directly
		if #nearbysystems > 0 then
			if ship.flightState == "LANDED" then
				ship:AIEnterLowOrbit(ship:FindNearestTo("PLANET") or ship:FindeNearestTo("STAR"))
			end
			Timer:CallAt(Game.time + 5, function () ship:HyperjumpTo(nearbysystems[1]) end)
		else
			return
		end
	end
end


function sar_target.searchForTarget (mission)
	-- Measure distance to target every second until interaction distance reached.

	if mission.searching == true or mission.target == nil then return end
	mission.searching = true

	-- Counter to show messages only once and not every loop
	local message_counter = {INTERACTION_DISTANCE_REACHED = 1,
	                         PLEASE_LAND = 1}

	Timer:CallEvery(1, function ()

		-- abort if player is about to leave system, target ship is destroyed, or player leaves target frame
		if sar_target.player_leaving_system or not mission.target or Game.player.frameBody ~= mission.target.frameBody then
			mission.searching = false
			return true

		else
			-- if distance to target has not been reached keep searching
			if not targetInteractionDistance(mission) then
				if message_counter.INTERACTION_DISTANCE_REACHED == 0 then
					Comms.ImportantMessage(l.INTERACTION_ABORTED)
					message_counter.INTERACTION_DISTANCE_REACHED = 1
					message_counter.PLEASE_LAND = 1
				end
				return false

				-- if distance to target has been reached start target interaction
			else
				if message_counter.INTERACTION_DISTANCE_REACHED > 0 then
					Comms.ImportantMessage(l.INTERACTION_DISTANCE_REACHED)
					message_counter.INTERACTION_DISTANCE_REACHED = 0
				end

				-- if planet-based mission require player to land
				if mission.flavour.loctype == "CLOSE_PLANET" or
				mission.flavour.loctype == "MEDIUM_PLANET" then
					if Game.player.flightState ~= "LANDED" then
						if message_counter.PLEASE_LAND > 0 then
							Comms.ImportantMessage(l.PLEASE_LAND)
							message_counter.PLEASE_LAND = 0
						end
						return false
					end
				end

				-- if mission is overdue
				if Game.time > mission.due then
					Comms.ImportantMessage(l.SHIP_UNRESPONSIVE)
					return true

				else
					-- calculate and display total interaction time
					local packages
					packages = mission.pickup_crew + mission.pickup_pass +
						mission.deliver_crew + mission.deliver_pass
					for _,num in pairs(mission.pickup_comm) do
						packages = packages + num
					end
					for _,num in pairs(mission.deliver_comm) do
						packages = packages + num
					end
					local total_interaction_time = sar_config.target_interaction_time * packages
					total_interaction_time = string.format("%." .. (1 or 0) .. "f",
															total_interaction_time/60)
					local interaction_time_txt = string.interp(l.TRANSFER_TIME,
																{minutes = total_interaction_time})
					Comms.ImportantMessage(interaction_time_txt)

					-- start interaction with target ship and stop search
					interactWithTarget(mission)
					mission.searching = false
					return true
				end
			end
		end
	end)
end


function sar_target.discardShip (ship)
	-- Gracefully discard ship that is not needed any longer for the ship by either:
	-- 1. hyperjumping to populated system, or
	-- 2. hyperjumping to non-populated system, or
	-- 3. fly to high orbit and explode.
	local with_stations = true
	local nearbysystems = sar_utils.findNearbySystems(with_stations)
	local status, distance, fuel, duration = ship:GetHyperspaceDetails(Game.system.path, nearbysystems[1])
	if #nearbysystems > 0 and status == "OK" then
		Timer:CallAt(Game.time + Engine.rand:Integer(5,10), function ()
			ship:AIEnterLowOrbit(ship:FindNearestTo("PLANET") or ship:FindNearestTo("STAR"))
			Timer:CallAt(Game.time + 30, function () ship:HyperjumpTo(nearbysystems[1]) end)
		end)
	else
		with_stations = false
		nearbysystems = sar_utils.findNearbySystems(with_stations)
		status, distance, fuel, duration = ship:GetHyperspaceDetails(Game.system.path, nearbysystems[1])
		if #nearbysystems > 0 and status == "OK" then
			Timer:CallAt(Game.time + Engine.rand:Integer(5,10), function ()
				ship:AIEnterLowOrbit(ship:FindNearestTo("PLANET") or ship:FindNearestTo("STAR"))
				Timer:CallAt(Game.time + 30, function () ship:HyperjumpTo(nearbysystems[1]) end)
			end)
		else
			Timer:CallAt(Game.time + Engine.rand:Integer(5,10), function ()
				ship:AIEnterHighOrbit(ship:FindNearestTo("PLANET") or ship:FindNearestTo("STAR"))
				Timer:CallAt(Game.time + 600, function () ship:Explode() end)
			end)
		end
	end
end


return sar_target
