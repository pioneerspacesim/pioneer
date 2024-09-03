-- Copyright © 2008-2024 Pioneer Developers. See AUTHORS.txt for details
-- Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

local utils = require 'utils'
local Serializer = require 'Serializer'
--
-- Class: EquipSet
--
-- A container for a ship's equipment.

---@class EquipSet
---@field meta table
local EquipSet = utils.inherits(nil, "EquipSet")

EquipSet.default = {
	cargo=0,
	engine=1,
	laser_front=1,
	laser_rear=0,
	missile=0,
	ecm=1,
	radar=1,
	target_scanner=1,
	hypercloud=1,
	hull_autorepair=1,
	energy_booster=1,
	atmo_shield=1,
	cabin=50,
	shield=9999,
	scoop=2,
	laser_cooler=1,
	cargo_life_support=1,
	autopilot=1,
	trade_computer=1,
	sensor = 2,
	thruster = 1
}

function EquipSet.New (slots)
	---@class EquipSet
	local obj = {}
	obj.slots = {}
	for k, n in pairs(EquipSet.default) do
		obj.slots[k] = {__occupied = 0, __limit = n}
	end
	for k, n in pairs(slots) do
		obj.slots[k] = {__occupied = 0, __limit = n}
	end
	setmetatable(obj, EquipSet.meta)
	return obj
end

local listeners = {}
function EquipSet:AddListener(listener)
	listeners[self] = listeners[self] or {}
	table.insert(listeners[self], listener)
end

function EquipSet:CallListener(slot)
	if not listeners[self] then return end

	for _, listener in ipairs(listeners[self]) do
		listener(slot)
	end
end

function EquipSet:Serialize()
	local serialize = {
		slots = {}
	}

	for k, v in pairs(self.slots) do
		serialize.slots[k] = v
	end

	return serialize
end

function EquipSet.Unserialize(data)
	setmetatable(data, EquipSet.meta)
	return data
end

--
-- Group: Methods
--

--
-- Method: FreeSpace
--
--  returns the available space in the given slot.
--
-- Parameters:
--
--  slot - The slot name.
--
-- Return:
--
--  free_space - The available space (integer)
--
function EquipSet:FreeSpace (slot)
	local s = self.slots[slot]
	if not s then
		return 0
	end
	return s.__limit - s.__occupied
end

function EquipSet:SlotSize(slot)
	local s = self.slots[slot]
	if not s then
		return 0
	end
	return s.__limit
end

--
-- Method: OccupiedSpace
--
--  returns the space occupied in the given slot.
--
-- Parameters:
--
--  slot - The slot name.
--
-- Return:
--
--  occupied_space - The occupied space (integer)
--
function EquipSet:OccupiedSpace (slot)
	local s = self.slots[slot]
	if not s then
		return 0
	end
	return s.__occupied
end

--
-- Method: Count
--
--  returns the number of occurrences of the given equipment in the specified slot.
--
-- Parameters:
--
--  item - The equipment to count.
--
--  slots - List of the slots to check. You can also provide a string if it
--          is only one slot. If this argument is not provided, all slots
--          will be searched.
--
-- Return:
--
--  free_space - The available space (integer)
--
function EquipSet:Count(item, slots)
	local to_check
	if type(slots) == "table" then
		to_check = {}
		for _, s in ipairs(slots) do
			table.insert(to_check, self.slots[s])
		end
	elseif slots == nil then
		to_check = self.slots
	else
		to_check = {self.slots[slots]}
	end

	local count = 0
	for _, slot in pairs(to_check) do
		for _, e in pairs(slot) do
			if e == item then
				count = count + 1
			end
		end
	end
	return count
end

function EquipSet:__TriggerCallbacks(ship, slot)
	ship:UpdateEquipStats()
	-- if we reduce the available capacity, we need to update the maximum amount of cargo available
	ship:setprop("totalCargo", math.min(self.slots.cargo.__limit, ship.usedCargo+ship.freeCapacity))
	self:CallListener(slot)
end

-- Method: __Remove_NoCheck (PRIVATE)
--
--  Remove equipment without checking whether the slot is appropriate nor
--  calling the uninstall hooks nor even checking the arguments sanity.
--  It DOES check the free place in the slot.
--
-- Parameters:
--
--  Please refer to the Remove method.
--
-- Return:
--
--  Please refer to the Remove method.
--
function EquipSet:__Remove_NoCheck (item, num, slot)
	local s = self.slots[slot]
	if not s or s.__occupied == 0 then
		return 0
	end
	local removed = 0
	for i = 1,s.__limit do
		if removed >= num or s.__occupied <= 0 then
			return removed
		end
		if s[i] == item then
			s[i] = nil
			removed = removed + 1
			s.__occupied = s.__occupied - 1
		end
	end
	return removed
end

-- Method: __Add_NoCheck (PRIVATE)
--
--  Add equipment without checking whether the slot is appropriate nor
--  calling the install hooks nor even checking the arguments sanity.
--  It DOES check the free place in the slot.
--
-- Parameters:
--
--  Please refer to the Add method.
--
-- Return:
--
--  Please refer to the Add method.
--
function EquipSet:__Add_NoCheck(item, num, slot)
	if self:FreeSpace(slot) == 0 then
		return 0
	end
	local s = self.slots[slot]
	local added = 0
	for i = 1,s.__limit do
		if added >= num or s.__occupied >= s.__limit then
			return added
		end
		if not s[i] then
			s[i] = item
			added = added + 1
			s.__occupied = s.__occupied + 1
		end
	end
	return added
end

-- Method: Add
--
--  Add some equipment to the set, filling the specified slot as much as
--  possible.
--
-- Parameters:
--
--  item - the equipment to install
--  num - the number of pieces to install. If nil, only one will be installed.
--  slot - the slot where to install the equipment. It will be checked against
--         the equipment itself, the method will return -1 if the slot isn't
--         valid. If nil, the default slot for the equipment will be used.
--
-- Return:
--
--  installed - the number of pieces actually installed, or -1 if the specified
--              slot is not valid.
--
function EquipSet:Add(ship, item, num, slot)
	num = num or 1
	if not slot then
		slot = item:GetDefaultSlot(ship)
	elseif not item:IsValidSlot(slot, ship) then
		return -1
	end
	assert(slot ~= "cargo", "Cargo slots for equipment are no longer valid")

	local added = self:__Add_NoCheck(item, num, slot)
	if added == 0 then
		return 0
	end
	local postinst_diff = added - item:Install(ship, added, slot)
	if postinst_diff > 0 then
		self:__Remove_NoCheck(item, postinst_diff, slot)
		added = added-postinst_diff
	end
	if added > 0 then
		self:__TriggerCallbacks(ship, slot)
	end
	return added
end

-- Method: Remove
--
--  Remove some equipment from the set.
--
-- Parameters:
--
--  item - the equipment to remove.
--  num - the number of pieces to uninstall. If nil, only one will be removed.
--  slot - the slot where to install the equipment. If nil, the default slot
--         for the equipment will be used.
--
-- Return:
--
--  removed - the number of pieces actually removed.
--
function EquipSet:Remove(ship, item, num, slot)
	num = num or 1
	if not slot then
		slot = item:GetDefaultSlot(ship)
	end
	assert(slot ~= "cargo", "Cargo slots for equipment are no longer valid")

	local removed = self:__Remove_NoCheck(item, num, slot)
	if removed == 0 then
		return 0
	end
	local postuninstall_diff = removed - item:Uninstall(ship, removed, slot)
	if postuninstall_diff > 0 then
		self:__Add_NoCheck(item, postuninstall_diff, slot)
		removed = removed-postuninstall_diff
	end
	if removed > 0 then
		self:__TriggerCallbacks(ship, slot)
	end
	return removed
end

local EquipSet__ClearSlot = function (self, ship, slot)
	local s = self.slots[slot]
	local item_counts = {}
	for k,v in pairs(s) do
		if type(k) == 'number' then
			item_counts[v] = (item_counts[v] or 0) + 1
		end
	end
	for item, count in pairs(item_counts) do
		local uninstalled = item:Uninstall(ship, count, slot)
		-- FIXME support failed uninstalls??
		-- note that failed uninstalls are almost incompatible with Ship::SetShipType
		assert(uninstalled == count)
	end
	self.slots[slot] = {__occupied = 0, __limit = s.__limit}
	self:__TriggerCallbacks(ship, slot)

end

function EquipSet:Clear(ship, slot_names)
	if slot_names == nil then
		for k,_ in pairs(self.slots) do
			EquipSet__ClearSlot(self, ship, k)
		end

	elseif type(slot_names) == 'string' then
		EquipSet__ClearSlot(self, ship, slot_names)

	elseif type(slot_names) == 'table' then
		for _, s in ipairs(slot_names) do
			EquipSet__ClearSlot(self, ship, s)
		end
	end
end

function EquipSet:Get(slot, index)
	if type(index) == "number" then
		return self.slots[slot][index]
	end
	local ret = {}
	for i,v in pairs(self.slots[slot]) do
		if type(i) == 'number' then
			ret[i] = v
		end
	end
	return ret
end

function EquipSet:Set(ship, slot_name, index, item)
	local slot = self.slots[slot_name]

	if index < 1 or index > slot.__limit then
		error("EquipSet:Set(): argument 'index' out of range")
	end

	local to_remove = slot[index]
	if item == to_remove then return end

	if not to_remove or to_remove:Uninstall(ship, 1, slot_name) == 1 then
		if not item or item:Install(ship, 1, slot_name) == 1 then
			if not item then
				slot.__occupied = slot.__occupied - 1
			elseif not to_remove then
				slot.__occupied = slot.__occupied + 1
			end
			slot[index] = item
			self:__TriggerCallbacks(ship, slot_name)
		else -- Rollback the uninstall
			if to_remove then to_remove:Install(ship, 1, slot_name) end
		end
	end
end
Serializer:RegisterClass("EquipSet", EquipSet)
return EquipSet
