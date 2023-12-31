-- Copyright © 2008-2024 Pioneer Developers. See AUTHORS.txt for details
-- Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

local ShipDef = require 'ShipDef'
local Serializer = require 'Serializer'

local utils = require 'utils'

--
-- Class: CargoManager
--
-- CargoManager represents and manages all ship-based cargo data storage, in
-- contrast to EquipSet which manages active ship equipment.
--

---@class CargoManager
local CargoManager = utils.class('CargoManager')

-- Constructor
--
-- Creates and initializes a CargoManager object for the given ship
function CargoManager:Constructor(ship)
	self.ship = ship

	-- Cargo space and cargo mass are the same thing right now
	self.usedCargoSpace = 0
	self.usedCargoMass = 0

	-- Initialize property variables on owning ship for backwards compatibility
	-- don't initialize them if they're already created after first save
	if not self.ship:hasprop("totalCargo") then
		ship:setprop("totalCargo", self:GetTotalSpace())
	end
	
	if not self.ship:hasprop("usedCargo") then
		ship:setprop("usedCargo", 0)
	end

	-- TODO: stored commodities should be represented as array of { name, count, meta } entries
	-- to allow for e.g. tracking stolen/scooped cargo, or special mission-related cargoes

	-- Commodity storage is implemented as simple hashtable of name -> { count=n } values
	-- to ease initial implementation
	---@type table<string, { count: number, life_support: number? }>
	self.commodities = {}

	-- Event listeners for changes to commodities stored in this manager
	self.listeners = {}
end

-- Method: OnShipTypeChanged
--
-- Reinitialize ship properties after changing the type of the ship
-- Note: cargo mass is not removed from ship.mass_cap when changing ship types
function CargoManager:OnShipTypeChanged()
	self.ship:setprop("totalCargo", self:GetTotalSpace())
	self.ship:setprop("usedCargo", self.usedCargoSpace)

	self.ship:UpdateEquipStats()
end

-- Method: GetFreeSpace
--
-- Returns the available amount of cargo space currently present on the vessel.
function CargoManager:GetFreeSpace()
	local ship = self.ship

	-- use mass_cap directly here instead of freeCapacity because this can be
	-- called before ship:UpdateEquipStats() has been called
	local avail_mass = ShipDef[ship.shipId].capacity - (ship.mass_cap or 0)
	local cargo_space = ShipDef[ship.shipId].equipSlotCapacity.cargo or 0

	return math.min(avail_mass, cargo_space - self.usedCargoSpace)
end

-- Method: GetUsedSpace
--
-- Returns the amount of cargo space currently occupied on the vessel.
function CargoManager:GetUsedSpace()
	return self.usedCargoSpace
end

-- Method: GetTotalSpace
--
-- Returns the maximum amount of cargo that could be stored on the vessel.
function CargoManager:GetTotalSpace()
	return self:GetFreeSpace() + self.usedCargoSpace
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
---@param type CommodityType
---@param count integer
function CargoManager:AddCommodity(type, count)
	-- TODO: use a cargo volume metric with variable mass instead of fixed 1m^3 == 1t
	local required_space = (type.mass or 1) * (count or 1)
	
	if self:GetFreeSpace() < required_space then
		return false
	end

	self.usedCargoSpace = self.usedCargoSpace + required_space
	self.ship:setprop("usedCargo", self.usedCargoSpace)

	self.usedCargoMass = self.usedCargoMass + required_space
	self.ship:setprop("mass_cap", self.ship.mass_cap + required_space)

	local storage = self.commodities[type.name]

	if not storage then
		storage = { count = 0, life_support = type.life_support }
		self.commodities[type.name] = storage
	end

	storage.count = storage.count + count

	self.ship:UpdateEquipStats()

	-- Notify listeners that the cargo contents have changed
	for _, fn in pairs(self.listeners) do
		fn(type, count)
	end

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
---@param type CommodityType
---@param count integer
function CargoManager:RemoveCommodity(type, count)
	local storage = self.commodities[type.name]

	if not storage or storage.count == 0 then
		return 0
	end

	local removed = math.min(storage.count, (count or 1))

	storage.count = storage.count - removed

	-- Remove the storage entry from the ship if the commodity is no longer stored here
	if storage.count == 0 then
		self.commodities[type.name] = nil
	end

	-- TODO: use a cargo volume metric with variable mass instead of fixed 1m^3 == 1t
	local freed_space = (type.mass or 1) * removed
	self.usedCargoSpace = self.usedCargoSpace - freed_space
	self.ship:setprop("usedCargo", self.usedCargoSpace)

	self.usedCargoMass = self.usedCargoMass - freed_space
	self.ship:setprop("mass_cap", self.ship.mass_cap - freed_space)

	self.ship:UpdateEquipStats()

	-- Notify listeners that the cargo contents have changed
	for _, fn in pairs(self.listeners) do
		fn(type, -removed)
	end

	return removed
end

-- Method: CountCommodity
--
-- Returns total amount of a commodity available in this cargo manager.
--
-- Parameters:
--   type - CommodityType object of the commodity to query
---@param type CommodityType
function CargoManager:CountCommodity(type)
	if not self.commodities[type.name] then
		return 0
	end

	return self.commodities[type.name].count
end

-- Method: DoLifeSupportChecks
--
-- Check for commodities on this vessel which would perish under the given
-- cargo life-support level.
-- Returns the name of the first commodity found that would perish.
---@param supportLevel integer
---@return string?
function CargoManager:DoLifeSupportChecks(supportLevel)
	for name, info in pairs(self.commodities) do
		if (info.life_support or 0) > supportLevel then
			return name
		end
	end

	return nil
end

-- Method: AddListener
--
-- Register a callback function to be notified when the cargo stored in this manager is changed.
-- The provided key will be used to uniquely identify the callback function and can be used to
-- later remove the event listener.
--
-- The callback function receives two arguments:
--
--   cargoType - an object describing the type of cargo that was added or removed. Usually a CommodityType.
--   count     - a number specifying how many items were added (positive) or removed (negative).
--
-- Parameters:
--   key - a unique value identifying the listener function being added.
--   fn - a callback function following the above format to be notified of any changes in cargo manifest.
function CargoManager:AddListener(key, fn)
	self.listeners[key] = fn
end

-- Method: RemoveListener
--
-- Remove a previously-added listener by providing the same key as was used to register it.
--
-- Parameters:
--   key - a unique value identifiying a listener function previously added.
function CargoManager:RemoveListener(key)
	self.listeners[key] = nil
end

function CargoManager:Serialize()
	return utils.filter_table(self, function(k, v) return k ~= "listeners" end)
end

function CargoManager:Unserialize()
	setmetatable(self, CargoManager.meta)
	self.listeners = {}
	
	return self
end

Serializer:RegisterClass('CargoManager', CargoManager)

return CargoManager
