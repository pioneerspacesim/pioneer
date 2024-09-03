-- Copyright © 2008-2024 Pioneer Developers. See AUTHORS.txt for details
-- Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

local utils = require 'utils'
local Serializer = require 'Serializer'
local Lang = require 'Lang'
local ShipDef = require 'ShipDef'

local Game = package.core['Game']
local Space = package.core['Space']

local laser = {}
local hyperspace = {}
local misc = {}

--
-- Class: EquipType
--
-- A container for a ship's equipment.
--
-- Its constructor takes a table, the "specs". Mandatory fields are the following:
--  * l10n_key: the key to look up the name and description of
--          the object in a language-agnostic way
--  * l10n_resource: where to look up the aforementioned key. If not specified,
--          the system assumes "equipment-core"
--  * capabilities: a table of string->int, having at least "mass" as a valid key
--
-- All specs are copied directly within the object (even those I know nothing about),
-- but it is a shallow copy. This is particularly important for the capabilities, as
-- modifying the capabilities of one EquipType instance might modify them for other
-- instances if the same table was used for all (which is strongly discouraged by the
-- author, but who knows ? Some people might find it useful.)
--
--
local EquipType = utils.inherits(nil, "EquipType")

function EquipType.New (specs)
	local obj = {}
	for i,v in pairs(specs) do
		obj[i] = v
	end
	if not obj.l10n_resource then
		obj.l10n_resource = "equipment-core"
	end
	local l = Lang.GetResource(obj.l10n_resource)
	obj.volatile = {
		description = l:get(obj.l10n_key.."_DESCRIPTION") or "",
		name = l[obj.l10n_key] or ""
	}
	setmetatable(obj, EquipType.meta)
	if type(obj.slots) ~= "table" then
		obj.slots = {obj.slots}
	end
	return obj
end

function EquipType:Serialize()
	local tmp = EquipType.Super().Serialize(self)
	local ret = {}
	for k,v in pairs(tmp) do
		if type(v) ~= "function" then
			ret[k] = v
		end
	end

	ret.volatile = nil
	return ret
end

function EquipType.Unserialize(data)
	local obj = EquipType.Super().Unserialize(data)
	setmetatable(obj, EquipType.meta)
	if not obj.l10n_resource then
		obj.l10n_resource = "equipment-core"
	end
	local l = Lang.GetResource(obj.l10n_resource)
	obj.volatile = {
		description = l:get(obj.l10n_key.."_DESCRIPTION") or "",
		name = l[obj.l10n_key] or ""
	}
	return obj
end

--
-- Group: Methods
--

--
-- Method: GetDefaultSlot
--
--  returns the default slot for this equipment
--
-- Parameters:
--
--  ship (optional) - if provided, tailors the answer for this specific ship
--
-- Return:
--
--  slot_name - A string identifying the slot.
--
function EquipType:GetDefaultSlot(ship)
	return self.slots[1]
end

--
-- Method: IsValidSlot
--
--  tells whether the given slot is valid for this equipment
--
-- Parameters:
--
--  slot - a string identifying the slot in question
--
--  ship (optional) - if provided, tailors the answer for this specific ship
--
-- Return:
--
--  valid - a boolean qualifying the validity of the slot.
--
function EquipType:IsValidSlot(slot, ship)
	for _, s in ipairs(self.slots) do
		if s == slot then
			return true
		end
	end
	return false
end

function EquipType:GetName()
	return self.volatile.name
end

function EquipType:GetDescription()
	return self.volatile.description
end

local function __ApplyMassLimit(ship, capabilities, num)
	if num <= 0 then return 0 end
	-- we need to use mass_cap directly (not, eg, ship.freeCapacity),
	-- because ship.freeCapacity may not have been updated when Install is called
	-- (see implementation of EquipSet:Set)
	local avail_mass = ShipDef[ship.shipId].capacity - (ship.mass_cap or 0)
	local item_mass = capabilities.mass or 0
	if item_mass > 0 then
		num = math.min(num, math.floor(avail_mass / item_mass))
	end
	return num
end

local function __ApplyCapabilities(ship, capabilities, num, factor)
	if num <= 0 then return 0 end
	factor = factor or 1
	for k,v in pairs(capabilities) do
		local full_name = k.."_cap"
		local prev = (ship:hasprop(full_name) and ship[full_name]) or 0
		ship:setprop(full_name, (factor*v*num)+prev)
	end
	return num
end

function EquipType:Install(ship, num, slot)
	local caps = self.capabilities
	num = __ApplyMassLimit(ship, caps, num)
	return __ApplyCapabilities(ship, caps, num, 1)
end

function EquipType:Uninstall(ship, num, slot)
	return __ApplyCapabilities(ship, self.capabilities, num, -1)
end

-- Base type for weapons
local LaserType = utils.inherits(EquipType, "LaserType")
function LaserType:Install(ship, num, slot)
	if num > 1 then num = 1 end -- FIXME: support installing multiple lasers (e.g., in the "cargo" slot?)
	if LaserType.Super().Install(self, ship, 1, slot) < 1 then return 0 end
	local prefix = slot..'_'
	for k,v in pairs(self.laser_stats) do
		ship:setprop(prefix..k, v)
	end
	return 1
end

function LaserType:Uninstall(ship, num, slot)
	if num > 1 then num = 1 end -- FIXME: support uninstalling multiple lasers (e.g., in the "cargo" slot?)
	if LaserType.Super().Uninstall(self, ship, 1) < 1 then return 0 end
	local prefix = (slot or "laser_front").."_"
	for k,v in pairs(self.laser_stats) do
		ship:unsetprop(prefix..k)
	end
	return 1
end

-- Single drive type, no support for slave drives.
local HyperdriveType = utils.inherits(EquipType, "HyperdriveType")

function HyperdriveType:GetMaximumRange(ship)
	return 625.0*(self.capabilities.hyperclass ^ 2) / (ship.staticMass + ship.fuelMassLeft)
end

-- range_max is as usual optional
function HyperdriveType:GetDuration(ship, distance, range_max)
	range_max = range_max or self:GetMaximumRange(ship)
	local hyperclass = self.capabilities.hyperclass
	return 0.36*distance^2/(range_max*hyperclass) * (86400*math.sqrt(ship.staticMass + ship.fuelMassLeft))
end

-- range_max is optional, distance defaults to the maximal range.
function HyperdriveType:GetFuelUse(ship, distance, range_max)
	range_max = range_max or self:GetMaximumRange(ship)
	local distance = distance or range_max
	local hyperclass_squared = self.capabilities.hyperclass^2
	return math.clamp(math.ceil(hyperclass_squared*distance / range_max), 1, hyperclass_squared);
end

-- if the destination is reachable, returns: distance, fuel, duration
-- if the destination is out of range, returns: distance
-- if the specified jump is invalid, returns nil
function HyperdriveType:CheckJump(ship, source, destination)
	if ship:GetEquip('engine', 1) ~= self or source:IsSameSystem(destination) then
		return nil
	end
	local distance = source:DistanceTo(destination)
	local max_range = self:GetMaximumRange(ship) -- takes fuel into account
	if distance > max_range then
		return distance
	end
	local fuel = self:GetFuelUse(ship, distance, max_range) -- specify range_max to avoid unnecessary recomputing.

	local duration = self:GetDuration(ship, distance, max_range) -- same as above
	return distance, fuel, duration
end

-- like HyperdriveType.CheckJump, but uses Game.system as the source system
-- if the destination is reachable, returns: distance, fuel, duration
-- if the destination is out of range, returns: distance
-- if the specified jump is invalid, returns nil
function HyperdriveType:CheckDestination(ship, destination)
	if not Game.system then
		return nil
	end
	return self:CheckJump(ship, Game.system.path, destination)
end

-- Give the range for the given remaining fuel
-- If the fuel isn't specified, it takes the current value.
function HyperdriveType:GetRange(ship, remaining_fuel)
	local range_max = self:GetMaximumRange(ship)
	local fuel_max = self:GetFuelUse(ship, range_max, range_max)

	---@type CargoManager
	local cargoMgr = ship:GetComponent('CargoManager')
	remaining_fuel = remaining_fuel or cargoMgr:CountCommodity(self.fuel)

	if fuel_max <= remaining_fuel then
		return range_max, range_max
	end
	local range = range_max*remaining_fuel/fuel_max

	while range > 0 and self:GetFuelUse(ship, range, range_max) > remaining_fuel do
		range = range - 0.05
	end

	-- range is never negative
	range = math.max(range, 0)
	return range, range_max
end

local HYPERDRIVE_SOUNDS_NORMAL = {
	warmup = "Hyperdrive_Charge",
	abort = "Hyperdrive_Abort",
	jump = "Hyperdrive_Jump",
}

local HYPERDRIVE_SOUNDS_MILITARY = {
	warmup = "Hyperdrive_Charge_Military",
	abort = "Hyperdrive_Abort_Military",
	jump = "Hyperdrive_Jump_Military",
}

function HyperdriveType:HyperjumpTo(ship, destination)
	-- First off, check that this is the primary engine.
	local engines = ship:GetEquip('engine')
	local primary_index = 0
	for i,e in ipairs(engines) do
		if e == self then
			primary_index = i
			break
		end
	end
	if primary_index == 0 then
		-- wrong ship
		return "WRONG_SHIP"
	end
	local distance, fuel_use, duration = self:CheckDestination(ship, destination)
	if not distance then
		return "OUT_OF_RANGE"
	end
	if not fuel_use then
		return "INSUFFICIENT_FUEL"
	end
	ship:setprop('nextJumpFuelUse', fuel_use)
	local warmup_time = 5 + self.capabilities.hyperclass*1.5

	local sounds
	if self.fuel.name == 'military_fuel' then
		sounds = HYPERDRIVE_SOUNDS_MILITARY
	else
		sounds = HYPERDRIVE_SOUNDS_NORMAL
	end

	return ship:InitiateHyperjumpTo(destination, warmup_time, duration, sounds), fuel_use, duration
end

function HyperdriveType:OnLeaveHyperspace(ship)
	if ship:hasprop('nextJumpFuelUse') then
		---@type CargoManager
		local cargoMgr = ship:GetComponent('CargoManager')

		local amount = ship.nextJumpFuelUse
		cargoMgr:RemoveCommodity(self.fuel, amount)
		if self.byproduct then
			cargoMgr:AddCommodity(self.byproduct, amount)
		end
		ship:unsetprop('nextJumpFuelUse')
	end
end

-- NOTE: "sensors" have no general-purpose code associated with the equipment type
local SensorType = utils.inherits(EquipType, "SensorType")

-- NOTE: all code related to managing a body scanner is implemented in the ScanManager component
local BodyScannerType = utils.inherits(SensorType, "BodyScannerType")

Serializer:RegisterClass("LaserType", LaserType)
Serializer:RegisterClass("EquipType", EquipType)
Serializer:RegisterClass("HyperdriveType", HyperdriveType)
Serializer:RegisterClass("SensorType", SensorType)
Serializer:RegisterClass("BodyScannerType", BodyScannerType)

return {
	laser			= laser,
	hyperspace		= hyperspace,
	misc			= misc,
	EquipType		= EquipType,
	LaserType		= LaserType,
	HyperdriveType	= HyperdriveType,
	SensorType		= SensorType,
	BodyScannerType	= BodyScannerType
}
