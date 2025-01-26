-- Copyright © 2008-2025 Pioneer Developers. See AUTHORS.txt for details
-- Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

local HullConfig = require 'HullConfig'
local Serializer = require 'Serializer'

local utils = require 'utils'

-- Class: EquipSet
--
-- EquipSet is responsible for managing all installed ship equipment items.
-- It provides helpers to query installed items of specific types, and
-- centralizes the management of "capability properties" set on the owning ship.

---@class EquipSet
---@field New fun(ship: Ship): EquipSet
local EquipSet = utils.class("EquipSet")

---@alias EquipSet.Listener fun(op: 'install'|'remove'|'modify', equip: EquipType, slot: HullConfig.Slot?)

-- Function: SlotTypeMatches
--
-- Static helper function that performs filtering of slot type identifiers.
--
-- Call this function when you want to know if the actual type of some object
-- is a valid match with the given filter string.
--
-- Example:
--
--  > local validEquipForSlot = EquipSet.SlotTypeMatches(equip.slot.type, slot.type)
--
-- Parameters:
--
--  actualType - string, concrete fully-qualified type string of the object or
--               slot in question
--
--  filter     - string, the partially-qualified type to check the concrete type
--               against to determine a match
--
---@param actualType string
---@param filter string
local function slotTypeMatches(actualType, filter)
	return actualType == filter or string.sub(actualType, 1, #filter + 1) == filter .. "."
end

EquipSet.SlotTypeMatches = slotTypeMatches

-- Function: CompatibleWithSlot
--
-- Static helper function to check if the given equipment item is compatible
-- with the given slot object. Validates type and size parameters of the slot.
--
-- Parameters:
--
--  equip - EquipType, equipment item instance or prototype to check
--
--  slot  - optional HullConfig.Slot, the slot the equipment item is being
--          validated against. If not present, the function validates that the
--          passed equipment item is a non-slot equipment item.
--
---@param equip EquipType
---@param slot HullConfig.Slot?
function EquipSet.CompatibleWithSlot(equip, slot)
	local equipSlot = equip.slot or false
	if not slot then return not equipSlot end

	return equipSlot and
		slotTypeMatches(equipSlot.type, slot.type)
		and (equipSlot.size <= slot.size)
		and (equipSlot.size >= (slot.size_min or slot.size))
end

-- Constructor: New
--
-- Construct a new EquipSet object for the given ship.
---@param ship Ship
function EquipSet:Constructor(ship)
	self.ship = ship
	self.config = HullConfig.GetHullConfig(ship.shipId)

	-- Stores a mapping of slot id -> equipment item
	-- Non-slot equipment is stored in the array portion.
	self.installed = {} ---@type table<string|integer, EquipType>
	-- NOTE: the integer value stored in the cache is NOT the current array
	-- index of the given item. It's simply a non-nil integer to indicate the
	-- item is not installed in a slot.
	self.cache = {} ---@type table<EquipType, string|integer>
	-- This provides a unique index value for non-slot equipment cache entries.
	-- It is monotonically increasing and prevents ID collisions for recursive
	-- slots provided by non-slot equipment items.
	self.cacheIndex = 1

	-- Stores a mapping of slot id -> slot handle
	-- Simplifies slot lookup for slots defined on equipment items
	self.slotCache = {} ---@type table<string, HullConfig.Slot>
	-- Stores the inverse mapping for looking up the compound id of a slot by
	-- the slot object itself.
	self.idCache = {} ---@type table<HullConfig.Slot, string>

	self:BuildSlotCache()

	-- List of listener functions to be notified of changes to the set of
	-- installed equipment in this EquipSet
	self.listeners = {} ---@type EquipSet.Listener[]

	-- Initialize ship properties we're responsible for modifying
	self.ship:setprop("mass_cap", self.ship["mass_cap"] or 0)
	self.ship:setprop("equipVolume", self.ship.equipVolume or 0)
	self.ship:setprop("totalVolume", self.ship.totalVolume or self.config.equipCapacity)
end

-- Callback: OnShipTypeChanged
--
-- Called when the type of the owning ship is changed. Removes all installed
-- equipment and tries to leave the ship in an "empty" state.
function EquipSet:OnShipTypeChanged()
	---@type (string|integer)[]
	local to_remove = {}

	for k in pairs(self.installed) do
		table.insert(to_remove, k)
	end

	-- Sort the longest strings first, as equipment installed in subslots will need to be removed first
	table.sort(to_remove, function(a, b)
		if type(a) == type(b) then
			return type(a) == "string" and #a > #b or type(a) == "number" and a > b
		else
			return type(a) == "string"
		end
	end)

	for _, id in ipairs(to_remove) do
		local equip = self.installed[id]
		self:Remove(equip)
	end

	assert(#self.installed == 0, "Missed some equipment while cleaning the ship")

	-- HullConfig changed, need to reset volume and rebuild list of slots
	self.config = HullConfig.GetHullConfig(self.ship.shipId)
	self.ship:setprop("totalVolume", self.config.equipCapacity)

	self.slotCache = {}
	self.idCache = {}

	self:BuildSlotCache()
end

-- Method: GetFreeVolume
--
-- Returns the available volume for mounting equipment
---@return number
function EquipSet:GetFreeVolume()
	return self.ship.totalVolume - self.ship.equipVolume
end

-- Method: GetSlotHandle
--
-- Return a reference to the slot with the given ID managed by this EquipSet.
-- The returned slot should be considered immutable.
--
-- Parameters:
--
--  id - string, fully-qualified ID of the slot to look up.
--
-- Returns:
--
--  slot - HullConfig.Slot?, the slot associated with that id if present or nil.
--
---@param id string
---@return HullConfig.Slot?
function EquipSet:GetSlotHandle(id)
	return self.slotCache[id]
end

-- Method: GetItemInSlot
--
-- Return the equipment item installed in the given slot, if present.
--
-- Parameters:
--
--  slot - HullConfig.Slot, a slot instance present on this ship.
--         Should come from config.slots or an installed equipment instance's
--         provides_slots field.
--
-- Returns:
--
--  equip - EquipType?, the equipment instance installed in the given
--          slot if any.
--
---@param slot HullConfig.Slot
---@return EquipType?
function EquipSet:GetItemInSlot(slot)
	-- The equipment item is not stored in the slot itself to reduce savefile
	-- size. While the API would be marginally simpler if so, there would be a
	-- significant amount of (de)serialization overhead as every ship and
	-- equipment instance would need to own an instance of the slot.
	local id = self.idCache[slot]
	return id and self.installed[id]
end

-- Method: GetFreeSlotForEquip
--
-- Attempts to find an available slot where the passed equipment item instance
-- could be installed.
--
-- Does not attempt to find the most optimal slot - the first slot which meets
-- the type and size constraints for the equipment item is returned.
--
-- The list of slots is iterated in undefined order.
--
-- Parameters:
--
--  equip - EquipType, the equipment item instance to attempt to slot.
--
-- Returns:
--
--  slot - HullConfig.Slot?, a valid slot for the item or nil.
--
---@param equip EquipType
---@return HullConfig.Slot?
function EquipSet:GetFreeSlotForEquip(equip)
	if not equip.slot then return nil end

	local filter = function(id, slot)
		return not self.installed[id]
			and slot.hardpoint == equip.slot.hardpoint
			and self:CanInstallInSlot(slot, equip)
	end

	for id, slot in pairs(self.slotCache) do
		if filter(id, slot) then
			return slot
		end
	end

	return nil
end

-- Method: GetAllSlotsOfType
--
-- Return a list of all slots matching the given filter parameters.
-- If hardpoint is not specified, returns both hardpoint and internal slots.
--
-- Parameters:
--
--  type      - string, slot type filter internally passed to SlotTypeMatches.
--
--  hardpoint - optional boolean, constrains the lost of slots to only those
--              which have a `hardpoint` key of the given type.
--
-- Returns:
--
--  slots - HullConfig.Slot[], unsorted list of slots which match the given
--          search criteria.
--
---@param type string
---@param hardpoint boolean?
---@return HullConfig.Slot[]
function EquipSet:GetAllSlotsOfType(type, hardpoint)
	local t = {}

	for _, slot in pairs(self.slotCache) do
		local match = (hardpoint == nil or hardpoint == slot.hardpoint)
			and slotTypeMatches(slot.type, type)
		if match then table.insert(t, slot) end
	end

	return t
end

-- Method: GetInstalledWithFilter
--
-- Return a list of all installed equipment of the given EquipType class matching the filter function
--
-- Parameters:
--
--  typename - string, constrains the results to only equipment items
--             inheriting from the given class.
--
--  filter   - function, returns a boolean indicating whether the equipment
--             item should be included in the returned list.
--
-- Returns:
--
--  items - EquipType[], list of items of the passed class which were accepted
--          by the filter function.
--
---@generic T : EquipType
---@param typename `T`
---@param filter fun(equip: T): boolean
---@return T[]
function EquipSet:GetInstalledWithFilter(typename, filter)
	local out = {}

	for _, equip in pairs(self.installed) do
		if equip:IsA(typename) and filter(equip) then
			table.insert(out, equip)
		end
	end

	return out
end

-- Method: GetInstalledOfType
--
-- Return a list of all installed equipment matching the given slot type
--
-- Parameters:
--
--  type - string, slot type filter string according to SlotTypeMatches
--
-- Returns:
--
--  installed - EquipType[], list of installed equipment which passed the
--              given slot type filter
--
---@param type string type filter
---@return EquipType[]
function EquipSet:GetInstalledOfType(type)
	local out = {}

	for _, equip in pairs(self.installed) do
		if equip.slot and slotTypeMatches(equip.slot.type, type) then
			table.insert(out, equip)
		end
	end

	return out
end

-- Method: GetInstalledEquipment
--
-- Returns a table containing all equipment items installed on this ship,
-- including both slot-based equipment and freely-installed equipment.
--
-- The returned table has both string and integer keys but should only be
-- iterated via `pairs()` as the integer keys are not guaranteed to be
-- contiguous.
---@return table<string|integer, EquipType>
function EquipSet:GetInstalledEquipment()
	return self.installed
end

-- Method: GetInstalledNonSlot
--
-- Returns an array containing all non-slot equipment items installed on this
-- ship. Items installed in a specific slot are not returned.
---@return EquipType[]
function EquipSet:GetInstalledNonSlot()
	local out = {}

	for i, equip in ipairs(self.installed) do
		out[i] = equip
	end

	return out
end

-- Method: CanInstallInSlot
--
-- Checks if the given equipment item could potentially fit in the passed slot,
-- given the current state of the ship.
--
-- If there is an item in the current slot, validates the fit as though that
-- item were not currently installed.
-- This function does not recurse into sub-slots provided by the item and thus
-- may not take into account the state of the ship if the equipped item and all
-- of its contained children were removed.
--
-- Parameters:
--
--  slotHandle - HullConfig.Slot, the slot to test the equipment item against.
--
--  equipment  - EquipType, the equipment item instance being checked for
--               installation.
--
-- Returns:
--
--  valid - boolean, indicates whether the given item could be installed in the
--          passed slot. Will always return false if the object is not a
--          slot-mounted item.
--
---@param slotHandle HullConfig.Slot
---@param equipment EquipType
function EquipSet:CanInstallInSlot(slotHandle, equipment)
	local equipped = self:GetItemInSlot(slotHandle)
	local freeVolume = self:GetFreeVolume() + (equipped and equipped.volume or 0)

	return (equipment.slot or false)
		and EquipSet.CompatibleWithSlot(equipment, slotHandle)
		and (freeVolume >= equipment.volume)
end

-- Method: CanInstallLoose
--
-- Checks if the given equipment item can be installed in the free equipment
-- volume of the ship. Returns false if the equipment item requires a slot.
---@param equipment EquipType
function EquipSet:CanInstallLoose(equipment)
	return not equipment.slot
		and self:GetFreeVolume() >= equipment.volume
end

-- Method: AddListener
--
-- Register an event listener function to be notified of changes to this ship's
-- equipment loadout.
---@param fun EquipSet.Listener
function EquipSet:AddListener(fun)
	table.insert(self.listeners, fun)
end

-- Method: RemoveListener
--
-- Remove a previously-registered event listener function.
function EquipSet:RemoveListener(fun)
	utils.remove_elem(self.listeners, fun)
end

-- Method: NotifyListeners
--
-- Send a notification event to listeners.
---@param op string The type of event to send.
---@param equipment EquipType The equipment item generating the event
---@param slot HullConfig.Slot? The slot handle on this EquipSet that is relevant to the event
function EquipSet:NotifyListeners(op, equipment, slot)
	for _, listener in ipairs(self.listeners) do
		listener(op, equipment, slot)
	end
end

-- Method: Install
--
-- Install an equipment item in the given slot or in free equipment volume.
--
-- Parameters:
--
--  equipment  - EquipType, an equipment item instance to install on this ship.
--               Must be an instance, not a equipment item prototype. Cannot be
--               currently installed on this or any other ship.
--
--  slotHandle - optional HullConfig.Slot, the slot to install the item into or
--               nil if the item does not support slot mounting. If present,
--               must be a slot returned from calling GetSlotHandle or a similar
--               function on this EquipSet instance.
--
-- Returns:
--
--  installed - boolean, indicates whether the item was successfully installed
--
---@param equipment EquipType
---@param slotHandle HullConfig.Slot?
---@return boolean
function EquipSet:Install(equipment, slotHandle)
	assert(equipment:isInstance())
	local slotId = self.idCache[slotHandle]

	if slotHandle then
		if not slotId then
			return false -- No such slot!
		end

		if self.installed[slotId] then
			return false -- Slot already full!
		end

		if not self:CanInstallInSlot(slotHandle, equipment) then
			return false -- Doesn't fit!
		end

		self.installed[slotId] = equipment
		self.cache[equipment] = slotId
	else
		if not self:CanInstallLoose(equipment) then
			return false
		end

		table.insert(self.installed, equipment)
		self.cache[equipment] = self.cacheIndex
		self.cacheIndex = self.cacheIndex + 1
	end

	equipment:OnInstall(self.ship, slotHandle)
	self:_InstallInternal(equipment)

	for _, fun in ipairs(self.listeners) do
		fun('install', equipment, slotHandle)
	end

	return true
end

-- Method: Remove
--
-- Remove a previously-installed equipment item from this ship.
--
-- Note that when removing an equipment item that provides slots, this function
-- will not recurse into an item's slots to remove installed sub-items.
--
-- All items installed into those slots must be manually removed before calling
-- this function.
--
-- Parameters:
--
--  equipment - EquipType, the equipment item to remove. Must be an equipment
--              item instance that was installed prior to this ship. Passing
--               an equipment prototype instance will not remove any equipment.
--
-- Returns:
--
--  removed - boolean, indicates successful removal of the item
--
---@param equipment EquipType
---@return boolean
function EquipSet:Remove(equipment)
	local cacheKey = self.cache[equipment]

	if not cacheKey then
		return false
	end

	local slotHandle = nil

	if type(cacheKey) == "string" then
		slotHandle = self:GetSlotHandle(cacheKey)
	end

	equipment:OnRemove(self.ship, slotHandle)
	self:_RemoveInternal(equipment)

	self.cache[equipment] = nil

	if slotHandle then
		self.installed[cacheKey] = nil
	else
		utils.remove_elem(self.installed, equipment)
	end

	for _, fun in ipairs(self.listeners) do
		fun('remove', equipment, slotHandle)
	end

	return true
end

-- Update ship properties after installing an equipment item
---@param equipment EquipType
---@private
function EquipSet:_InstallInternal(equipment)
	self.ship:setprop("mass_cap", self.ship["mass_cap"] + equipment.mass)
	self.ship:setprop("equipVolume", self.ship.equipVolume + equipment.volume)

	if equipment.capabilities then
		for k, v in pairs(equipment.capabilities) do
			local cap = k .. "_cap"
			self.ship:setprop(cap, (self.ship:hasprop(cap) and self.ship[cap] or 0) + v)
		end
	end

	if equipment.provides_slots then
		local baseId = tostring(self.cache[equipment]) .. "##"
		for _, slot in pairs(equipment.provides_slots) do
			local slotId = baseId .. slot.id
			assert(not self.slotCache[slotId])

			self.slotCache[slotId] = slot
			self.idCache[slot] = slotId
		end
	end

	self.ship:UpdateEquipStats()
end

-- Update ship properties after removing an equipment item
---@param equipment EquipType
---@private
function EquipSet:_RemoveInternal(equipment)
	self.ship:setprop("mass_cap", self.ship["mass_cap"] - equipment.mass)
	self.ship:setprop("equipVolume", self.ship.equipVolume - equipment.volume)

	if equipment.provides_slots then
		for _, slot in pairs(equipment.provides_slots) do
			local slotId = self.idCache[slot]
			assert(slotId)

			self.slotCache[slotId] = nil
			self.idCache[slot] = nil
		end
	end

	if equipment.capabilities then
		for k, v in pairs(equipment.capabilities) do
			local cap = k .. "_cap"
			self.ship:setprop(cap, self.ship[cap] - v)
		end
	end

	self.ship:UpdateEquipStats()
end

-- Populate the slot cache
---@private
function EquipSet:BuildSlotCache()
	for _, slot in pairs(self.config.slots) do
		self.slotCache[slot.id] = slot
		self.idCache[slot] = slot.id
	end

	-- id is the (potentially compound) slot id the equipment is already installed in
	for id, equip in pairs(self.installed) do
		if equip.provides_slots then
			for _, slot in pairs(equip.provides_slots) do
				local slotId = tostring(id) .. "##" .. slot.id
				self.slotCache[slotId] = slot
				self.idCache[slot] = slotId
			end
		end
	end
end

-- Remove transient fields from the serialized copy of the EquipSet
function EquipSet:Serialize()
	local obj = table.copy(self)

	obj.cache = nil
	obj.slotCache = nil
	obj.idCache = nil
	obj.listeners = nil

	return obj
end

-- Restore transient fields to the unserialized version of the EquipSet
function EquipSet:Unserialize()
	self.cache = {}
	self.listeners = {}

	for k, v in pairs(self.installed) do
		self.cache[v] = k
	end

	self.slotCache = {}
	self.idCache = {}

	setmetatable(self, EquipSet.meta)

	self:BuildSlotCache()

	return self
end

Serializer:RegisterClass("EquipSet", EquipSet)

return EquipSet
