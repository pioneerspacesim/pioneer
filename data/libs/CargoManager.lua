-- Copyright © 2008-2022 Pioneer Developers. See AUTHORS.txt for details
-- Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

local ShipDef = require 'ShipDef'

local utils = require 'utils'

--
-- Class: CargoManager
--
-- CargoManager represents and manages all ship-based cargo data storage, in
-- contrast to EquipSet which manages active ship equipment.
--

local CargoManager = utils.class('CargoManager')

-- Constructor
--
-- Creates and initializes a CargoManager object for the given ship
function CargoManager:Constructor(ship)
	self.ship = ship

	-- Cargo space and cargo mass are the same thing right now
	self.usedCargoSpace = 0
	self.usedCargoMass = 0

	-- TODO: stored commodities should be represented as array of { name, count, meta } entries
	-- to allow for e.g. tracking stolen/scooped cargo, or special mission-related cargoes

	-- Commodity storage is implemented as simple hashtable of name -> { count=n } values
	-- to ease initial implementation
	self.commodities = {}
end

-- Method: GetFreeSpace
--
-- Returns the available amount of cargo space currently present on the vessel.
function CargoManager:GetFreeSpace()
	local ship = self.ship

	local avail_mass = ShipDef[ship.shipId].capacity - ship.mass_cap
	local cargo_slots = ship.equipSet.slots.cargo

	return math.min(avail_mass, cargo_slots.__limit - self.usedCargoSpace)
end

-- Method: GetTotalSpace
--
-- Returns the theoretical maximum amount of cargo that could be stored on the vessel.
function CargoManager:GetTotalSpace()
	local ship = self.ship
	return math.min(ShipDef[ship.shipId].capacity, ship.equipSet.slots.cargo.__limit)
end

-- Method: AddCommodity
--
-- Add a specific number of the given commodity to this cargo manager.
-- Will return false if the total number specified cannot be removed from the
-- cargo manager.
--
-- Parameters:
--   type - CommodityType object of the commodity to add
--   count - number of commodities
--
-- Returns:
--   success - boolean indicating whether there was enough space on the vessel
--             to store the commodity
function CargoManager:AddCommodity(type, count)
	-- TODO: use a cargo volume metric with variable mass instead of fixed 1m^3 == 1t
	local required_space = (type.mass or 1) * (count or 1)

	if self:GetFreeSpace() < required_space then
		return false
	end

	self.usedCargoSpace = self.usedCargoSpace + required_space

	self.usedCargoMass = self.usedCargoMass + required_space
	self.ship:setcap("mass_cap", self.ship.mass_cap + required_space)

	local storage = self.commodities[type.name]

	if not storage then
		storage = { count = 0 }
		self.commodities[type.name] = storage
	end

	storage.count = storage.count + count

	return true
end

-- Method: RemoveCommodity
--
-- Remove a specific number of the specified commodity from this cargo manager.
-- Will return the number of commodities removed, even if that number is less
-- than initially desired.
--
-- Parameters:
--   type - CommodityType object of the commodity to remove
--   count - maximum number of commodity items to remove
--
-- Returns:
--   numRemoved - total number of commodity items removed, or 0 if no items
--                were removed from the cargo
function CargoManager:RemoveCommodity(type, count)
	local storage = self.commodities[type.name]

	if not storage or storage.count == 0 then
		return 0
	end

	local removed = math.min(storage.count, (count or 1))

	storage.count = storage.count - removed

	-- TODO: use a cargo volume metric with variable mass instead of fixed 1m^3 == 1t
	local freed_space = (type.mass or 1) * removed
	self.usedCargoSpace = self.usedCargoSpace - freed_space

	self.usedCargoMass = self.usedCargoMass - freed_space
	self.ship:setcap("mass_cap", self.ship.mass_cap - freed_space)

	return removed
end

-- Method: CountCommodity
--
-- Returns total amount of a commodity available in this cargo manager.
--
-- Parameters:
--   type - CommodityType object of the commodity to query
function CargoManager:CountCommodity(type)
	if not self.commodities[type.name] then
		return 0
	end

	return self.commodities[type.name].count
end

return CargoManager
