--
-- Class: EquipSet
--
-- A container for a ship's equipment.
local EquipSet = {class="EquipSet"}
local equipSet_meta = { __index = EquipSet }

EquipSet.default = {
	cargo=0,
	engine=1,
	laser_front=1,
	laser_rear=0,
	missile=0,
	ecm=1,
	scanner=1,
	radar=1,
	hypercloud=1,
	hull_autorepair=1,
	energy_booster=1,
	atmo_shield=1,
	cabin=50,
	shield=9999,
	fuel_scoop=1,
	cargo_scoop=1,
	laser_cooler=1,
	cargo_life_support=1,
	autopilot=1,
}

function EquipSet.New (slots)
	obj = {}
	obj.slots = {}
	for k, n in pairs(EquipSet.default) do
		obj.slots[k] = {__occupied = 0, __limit = n}
	end
	for k, n in pairs(slots) do
		obj.slots[k] = {__occupied = 0, __limit = n}
	end
	setmetatable(obj, equipSet_meta)
	return obj
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

function EquipSet:SlotSize(slot)
	local s = self.slots[slot]
	if not s then
		return 0
	end
	return s.__limit
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
	if type(slots) == "table" then
		local to_check = {}
		for _, s in ipairs(slots) do
			table.insert(to_check, s)
		end
	elseif slots == nil then
		to_check = self.slots
	else
		to_check = {self.slots[slot]}
	end

	local count = 0
	for _, slot in pairs(to_check) do
		for _, e in ipairs(slot) do
			if e == item then
				count = count + 1
			end
		end
	end
	return count
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
	elseif not item:IsValidSlot(ship, slot) then
		return -1
	end

	local added = self:__Add_NoCheck(item, num, slot)
	if added == 0 then
		return 0
	end
	local postinst_diff = added - item:Install(ship, num, slot)
	if postinst_diff <= 0 then
		return added
	end

	self:__Remove_NoCheck(item, postinst_diff, slot)
	return added - postinst_diff
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
	local num = num or 1
	if not slot then
		local slot = item:GetDefaultSlot(ship)
	end
	local removed = self:__Remove_NoCheck(item, num, slot)
	if removed == 0 then
		return 0
	end
	local postuninstall_diff = removed - item:Uninstall(ship, num, slot)
	if postuninstall_diff <= 0 then
		return removed
	end

	self:__Add_NoCheck(item, postuninstall_diff, slot)
	return removed - postuninstall_diff
end

function EquipSet:Get(slot, index)
	if type(index) == "number" then
		return self.slots[slot][index]
	end
	ret = {}
	for i,v in ipairs(self.slots[slot]) do
		table.insert(ret, i, v)
	end
	return ret
end

function EquipSet:Set(ship, slot, index, item)
	if index < 1 or index > # (self.slots[slot]) then
		error("EquipSet:Set(): argument 'index' out of range")
	end
	to_remove = self.slots[slot][index]
	if not to_remove or to_remove:Uninstall(ship, 1, slot) == 1 then
		if item:Install(ship, 1, slot) == 1 then
			self.slots[slot][index] = item
		else -- Rollback the uninstall
			to_remove:Install(ship, 1, slot)
		end
	end
end

return EquipSet
