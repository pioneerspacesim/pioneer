-- Copyright © 2008-2025 Pioneer Developers. See AUTHORS.txt for details
-- Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

local Equipment = require 'Equipment'
local EquipSet = require 'EquipSet'

local utils = require 'utils'

-- Module: Passengers
--
-- Helper utilities for working with passengers onboard a ship
local Passengers = {}


-- Function: CountOccupiedCabins
--
-- Return the number of occupied passenger berths present on the ship
--
---@param ship Ship
---@return integer
function Passengers.CountOccupiedBerths(ship)
	return ship["cabin_occupied_cap"] or 0
end

-- Function: CountFreeCabins
--
-- Return the number of unoccupied passenger berths present on the ship
--
---@param ship Ship
---@return integer
function Passengers.CountFreeBerths(ship)
	return (ship["cabin_cap"] or 0) - (ship["cabin_occupied_cap"] or 0)
end

-- Function: GetOccupiedCabins
--
-- Return a list of currently occupied cabin equipment items
--
---@param ship Ship
---@return Equipment.CabinType[]
function Passengers.GetOccupiedCabins(ship)
	local equipSet = ship:GetComponent("EquipSet")

	return equipSet:GetInstalledWithFilter('Equipment.CabinType', function(equip)
		return equip:GetNumPassengers() > 0
	end)
end

-- Function: GetFreeCabins
--
-- Return a list of passenger cabins containing at least numBerths free
-- passenger berths. If not specified, numBerths defaults to 1.
--
---@param ship Ship
---@param numBerths integer?
---@return Equipment.CabinType[]
function Passengers.GetFreeCabins(ship, numBerths)
	local equipSet = ship:GetComponent("EquipSet")

	return equipSet:GetInstalledWithFilter('Equipment.CabinType', function(equip)
		return equip:GetFreeBerths() >= (numBerths or 1)
	end)
end

-- Function: CheckEmbarked
--
-- Validate how many of the given list of passengers are present on this ship
--
---@param ship Ship
---@param passengers Character[]
---@return integer
function Passengers.CheckEmbarked(ship, passengers)
	local occupiedCabins = Passengers.GetOccupiedCabins(ship)
	local passengerLookup = utils.map_table(passengers, function(_, passenger)
		return passenger, true
	end)

	local count = 0

	for _, cabin in ipairs(occupiedCabins) do
		for _, passenger in ipairs(cabin.passengers) do
			if passengerLookup[passenger] then
				count = count + 1
			end
		end
	end

	return count
end

-- Function: EmbarkPassenger
--
-- Load the given passenger onboard the ship, optionally to a specific cabin.
--
---@param ship Ship
---@param passenger Character
---@param cabin EquipType?
---@return boolean success
function Passengers.EmbarkPassenger(ship, passenger, cabin)
	---@cast cabin Equipment.CabinType?
	if not cabin then
		cabin = Passengers.GetFreeCabins(ship)[1]
	end

	if not cabin then
		return false
	end

	if cabin:GetFreeBerths() == 0 then
		return false
	end

	cabin:AddPassenger(passenger)
	ship:setprop("cabin_occupied_cap", (ship["cabin_occupied_cap"] or 0) + 1)

	return true
end

-- Function: DisembarkPassenger
--
-- Unload the given passenger from the ship. If not specified, the function
-- will attempt to automatically determine which cabin the passenger is
-- embarked in.
--
---@param ship Ship
---@param passenger Character
---@param cabin EquipType?
function Passengers.DisembarkPassenger(ship, passenger, cabin)
	---@cast cabin Equipment.CabinType?
	if not cabin then
		local equipSet = ship:GetComponent("EquipSet")

		---@type Equipment.CabinType[]
		local cabins = equipSet:GetInstalledWithFilter('Equipment.CabinType', function(equip)
			return equip:HasPassenger(passenger)
		end)

		cabin = cabins[1]
	else
		cabin = cabin:HasPassenger(passenger) and cabin or nil
	end

	if not cabin then
		return false
	end

	cabin:RemovePassenger(passenger)
	ship:setprop("cabin_occupied_cap", ship["cabin_occupied_cap"] - 1)

	return true
end

-- Function: GetMaxPassengersForHull
--
-- Determine the maximum number of passengers the passed HullConfig can
-- accommodate with its cabin slots.
--
-- Optionally takes a mass limit to constrain the total mass cost of cabins
-- the ship can be equipped with.
---@param hull HullConfig
---@param maxMass number?
---@return integer
function Passengers.GetMaxPassengersForHull(hull, maxMass)
	local numPassengers = 0
	local availMass = maxMass or math.huge

	---@type Equipment.CabinType
	local cabins = utils.to_array(Equipment.new, function(equip)
		return equip:IsA('Equipment.CabinType')
	end)

	-- Compute the theoretical maximum number of passengers
	for _, slot in pairs(hull.slots) do
		if EquipSet.SlotTypeMatches(slot.type, "cabin") then
			local cabin, passengers = utils.best_score(cabins, function(_, equip)
				return EquipSet.CompatibleWithSlot(equip, slot)
					and (availMass - equip.mass > 0)
					and equip.capabilities.cabin or nil
			end)
			if cabin then
				numPassengers = numPassengers + passengers
				availMass = availMass - cabin.mass
			end
		end
	end

	return numPassengers
end

return Passengers
