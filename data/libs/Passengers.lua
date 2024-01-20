-- Copyright © 2008-2023 Pioneer Developers. See AUTHORS.txt for details
-- Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

local Equipment = require 'Equipment'
local utils = require 'utils'

-- Module: Passengers
--
-- Helper utilities for working with passengers onboard a ship
local Passengers = {}


-- Function: CountOccupiedCabins
--
-- Return the number of occupied cabins present on the ship
--
---@param ship Ship
---@return integer
function Passengers.CountOccupiedCabins(ship)
	return ship["cabin_occupied_cap"] or 0
end

-- Function: CountFreeCabins
--
-- Return the number of unoccupied cabins present on the ship
--
---@param ship Ship
---@return integer
function Passengers.CountFreeCabins(ship)
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
	local cabin = Equipment.Get("misc.cabin")

	return equipSet:GetInstalledWithFilter(function(equip)
		return equip:GetPrototype() == cabin and equip.passenger
	end)
end

-- Function: GetFreeCabins
--
-- Return a list of currently free passenger cabins
--
---@param ship Ship
---@return Equipment.CabinType[]
function Passengers.GetFreeCabins(ship)
	local equipSet = ship:GetComponent("EquipSet")
	local cabin = Equipment.Get("misc.cabin")

	return equipSet:GetInstalledWithFilter(function(equip)
		return equip:GetPrototype() == cabin and equip.passenger == nil
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
	local passengerLookup = {}

	for i, cabin in ipairs(occupiedCabins) do
		passengerLookup[cabin.passenger] = true
	end

	local count = 0

	for i, passenger in ipairs(passengers) do
		if passengerLookup[passenger] then
			count = count + 1
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
function Passengers.EmbarkPassenger(ship, passenger, cabin)
	if not cabin then
		cabin = Passengers.GetFreeCabins(ship)[1]
	end

	if not cabin or cabin.passenger then
		return false
	end

	---@cast cabin Equipment.CabinType
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
	if not cabin then
		local cabinProto = Equipment.Get("misc.cabin")
		local equipSet = ship:GetComponent("EquipSet")

		cabin = equipSet:GetInstalledWithFilter(function(equip)
			return equip:GetPrototype() == cabinProto and equip.passenger == passenger
		end)[1]
	end

	if not cabin or cabin.passenger ~= passenger then
		return false
	end

	---@cast cabin Equipment.CabinType
	cabin:RemovePassenger(passenger)
	ship:setprop("cabin_occupied_cap", ship["cabin_occupied_cap"] - 1)

	return true
end

return Passengers
