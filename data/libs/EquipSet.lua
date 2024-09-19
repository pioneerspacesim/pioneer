-- Copyright © 2008-2023 Pioneer Developers. See AUTHORS.txt for details
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

local function slotTypeMatches(equipType, slotType)
	return equipType == slotType or string.sub(equipType, 1, #slotType + 1) == slotType .. "."
end

EquipSet.SlotTypeMatches = slotTypeMatches

-- Function: CompatibleWithSlot
--
-- Static helper function to check if the given equipment item is compatible
-- with the given slot object. Validates type and size parameters of the slot.
---@param equip EquipType
---@param slot HullConfig.Slot?
function EquipSet.CompatibleWithSlot(equip, slot)
	local equipSlot = equip.slot or false
	if not slot then return not equipSlot end

	return equipSlot and
		slotTypeMatches(equip.slot.type, slot.type)
		and (equip.slot.size <= slot.size)
		and (equip.slot.size >= (slot.size_min or slot.size))
end

---@param ship Ship
function EquipSet:Constructor(ship)
	self.ship = ship
	self.config = HullConfig.GetHullConfigs()[ship.shipId]

	-- Stores a mapping of slot id -> equipment item
	-- Non-slot equipment is stored in the array portion
	self.installed = {} ---@type table<string|integer, EquipType>
	-- Note: the integer value stored in the cache is NOT the current array
	-- index of the given item. It's simply a non-nil integer to indicate the
	-- item is not installed in a slot.
	self.cache = {} ---@type table<EquipType, string|integer>

	-- Stores a mapping of slot id -> slot handle
	-- Simplifies slot lookup for slots defined on equipment items
	self.slotCache = {} ---@type table<string, HullConfig.Slot>
	-- Stores the inverse mapping for looking up the compound id of a slot by
	-- the slot object itself.
	self.idCache = {} ---@type table<HullConfig.Slot, string>

	self:BuildSlotCache()

	self.listeners = {}

	-- Initialize ship properties we're responsible for modifying
	self.ship:setprop("mass_cap", self.ship["mass_cap"] or 0)
	self.ship:setprop("equipVolume", self.ship.equipVolume or 0)
	self.ship:setprop("totalVolume", self.ship.totalVolume or self.config.capacity)
end

-- Function: GetFreeVolume
--
-- Returns the available volume for mounting equipment
---@return number
function EquipSet:GetFreeVolume()
	return self.ship.totalVolume - self.ship.equipVolume
end

-- Function: GetSlotHandle
--
-- Return a reference to the slot with the given ID managed by this EquipSet.
-- The returned slot should be considered immutable.
---@param id string
---@return HullConfig.Slot?
function EquipSet:GetSlotHandle(id)
	return self.slotCache[id]
end

-- Function: GetItemInSlot
--
-- Return the equipment item installed in the given slot, if present.
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

-- Function: GetFreeSlotForEquip
--
-- Attempts to find an available slot where the passed equipment item could be
-- installed. Does not attempt to find the most optimal slot - the first slot
-- which meets the type and size constraints for the equipment item is returned.
---@param equip EquipType
---@return HullConfig.Slot?
function EquipSet:GetFreeSlotForEquip(equip)
	if not equip.slot then return nil end

	local filter = function(_, slot)
		return not slot.item
			and slot.hardpoint == equip.slot.hardpoint
			and self:CanInstallInSlot(slot, equip)
	end

	for _, slot in pairs(self.slotCache) do
		if filter(_, slot) then
			return slot
		end
	end

	return nil
end

-- Function: GetAllSlotsOfType
--
-- Return a list of all slots matching the given filter parameters.
-- If hardpoint is not specified, returns both hardpoint and internal slots.
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

-- Function: CountInstalledWithFilter
--
-- Return a count of all installed equipment matching the given filter function
---@param filter fun(equip: EquipType): boolean
---@return integer
function EquipSet:CountInstalledWithFilter(filter)
	local count = 0

	for _, equip in pairs(self.installed) do
		if filter(equip) then
			count = count + (equip.count or 1)
		end
	end

	return count
end

-- Function: GetInstalledWithFilter
--
-- Return a list of all installed equipment matching the given filter function
---@param filter fun(equip: EquipType): boolean
---@return EquipType[]
function EquipSet:GetInstalledWithFilter(filter)
	local out = {}

	for _, equip in pairs(self.installed) do
		if filter(equip) then
			table.insert(out, equip)
		end
	end

	return out
end

-- Function: GetInstalledOfType
--
-- Return a list of all installed equipment matching the given slot type
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

-- Function: GetInstalledEquipment
--
-- Returns a table containing all equipment items installed on this ship,
-- including both slot-based equipment and freely-installed equipment.
---@return table<string|integer, EquipType>
function EquipSet:GetInstalledEquipment()
	return self.installed
end

-- Function: GetInstalledNonSlot
--
-- Returns an array containing all non-slot equipment items installed on this
-- ship. Items installed in a specific slot are not returned.
--
-- Status:
--
--    experimental
--
---@return EquipType[]
function EquipSet:GetInstalledNonSlot()
	local out = {}

	for i, equip in ipairs(self.installed) do
		out[i] = equip
	end

	return out
end

-- Function: CanInstallInSlot
--
-- Checks if the given equipment item could potentially fit in the passed slot,
-- given the current state of the ship.
--
-- If there is an item in the current slot, validates the fit as though that
-- item were not currently installed.
-- Returns false if the equipment item is not compatible with slot mounting.
---@param slotHandle HullConfig.Slot
---@param equipment EquipType
function EquipSet:CanInstallInSlot(slotHandle, equipment)
	local equipped = self:GetItemInSlot(slotHandle)
	local freeVolume = self:GetFreeVolume() + (equipped and equipped.volume or 0)

	return (equipment.slot or false)
		and EquipSet.CompatibleWithSlot(equipment, slotHandle)
		and (freeVolume >= equipment.volume)
end

-- Function: CanInstallLoose
--
-- Checks if the given equipment item can be installed in the free equipment
-- volume of the ship. Returns false if the equipment item requires a slot.
---@param equipment EquipType
function EquipSet:CanInstallLoose(equipment)
	return not equipment.slot
		and self:GetFreeVolume() >= equipment.volume
end

-- Function: AddListener
--
-- Register an event listener function to be notified of changes to this ship's
-- equipment loadout.
function EquipSet:AddListener(fun)
	table.insert(self.listeners, fun)
end

-- Function: RemoveListener
--
-- Remove a previously-registered event listener function.
function EquipSet:RemoveListener(fun)
	utils.remove_elem(self.listeners, fun)
end

-- Function: Install
--
-- Install an equipment item in the given slot or in free equipment volume.
--
-- The passed equipment item must be an equipment item instance, not an
-- equipment item prototype.
--
-- The passed slot must be a slot returned from caling GetSlotHandle or
-- GetAllSlotsOfType on this EquipSet.
---@param equipment EquipType
---@param slotHandle HullConfig.Slot?
---@return boolean
function EquipSet:Install(equipment, slotHandle)
	print("Installing equip {} in slot {}" % { equipment:GetName(), slotHandle })
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
		self.cache[equipment] = #self.installed
	end

	equipment:OnInstall(self.ship, slotHandle)
	self:_InstallInternal(equipment)

	for _, fun in ipairs(self.listeners) do
		fun("install", equipment, slotHandle)
	end

	return true
end

-- Function: Remove
--
-- Remove a previously-installed equipment item from this ship.
--
-- The equipment item must be the same item instance that was installed prior;
-- passing an equipment prototype instance will not result in any equipment
-- item being removed.
--
-- Note that when removing an equipment item that provides slots, all items
-- installed into those slots must be manually removed *before* calling this
-- function! EquipSet:Remove() will not recurse into an item's slots to remove
-- installed sub-items!
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
		fun("remove", equipment, slotHandle)
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
