-- Copyright © 2008-2024 Pioneer Developers. See AUTHORS.txt for details
-- Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

local utils = require 'utils'
local Serializer = require 'Serializer'
local Lang = require 'Lang'

local Game = package.core['Game']

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
--  * capabilities: a table of string->number properties to set on the ship object.
--          All keys will be suffixed with _cap for namespacing/convience reasons.
--
-- All specs are copied directly within the object (even those I know nothing about),
-- but it is a shallow copy. This is particularly important for the capabilities, as
-- modifying the capabilities of one EquipType instance might modify them for other
-- instances if the same table was used for all (which is strongly discouraged by the
-- author, but who knows ? Some people might find it useful.)
--
--
---@class EquipType
---@field id string
---@field mass number
---@field volume number
---@field slot { type: string, size: integer, hardpoint: boolean } | nil
---@field capabilities table<string, number>?
---@field purchasable boolean
---@field price number
---@field icon_name string?
---@field tech_level integer | "MILITARY"
---@field transient table
---@field slots table -- deprecated
---@field count integer?
---@field provides_slots table<string, HullConfig.Slot>?
---@field __proto EquipType?
local EquipType = utils.inherits(nil, "EquipType")

---@return EquipType
function EquipType.New (specs)
	---@class EquipType
	local obj = {}
	for i,v in pairs(specs) do
		obj[i] = v
	end

	if not obj.l10n_resource then
		obj.l10n_resource = "equipment-core"
	end

	setmetatable(obj, EquipType.meta)
	EquipType._createTransient(obj)

	if type(obj.slots) ~= "table" then
		obj.slots = {obj.slots}
	end

	if obj.slot and not obj.slot.hardpoint then
		obj.slot.hardpoint = false
	end

	if not obj.tech_level then
		obj.tech_level = 1
	end

	if not obj.icon_name then
		obj.icon_name = "equip_generic"
	end

	if not obj.purchasable then
		obj.price = obj.price or 0
	end

	return obj
end

-- Method: SpecializeForShip
--
-- Override this with a function customizing the equipment instance for the passed ship
-- (E.g. for equipment with mass/volume/cost dependent on the specific ship hull).
--
-- Parameters:
--
--  ship - HullConfig, hull configuration this item is tailored for. Note that
--         the config may not be associated with a concrete Ship object yet.
--
EquipType.SpecializeForShip = nil ---@type nil | fun(self: self, ship: HullConfig)

function EquipType._createTransient(obj)
	local l = Lang.GetResource(obj.l10n_resource)
	obj.transient = {
		description = l:get(obj.l10n_key .. "_DESCRIPTION") or "",
		name = l[obj.l10n_key] or ""
	}
end

-- Method: OnInstall
--
-- Perform any setup associated with installing this item on a Ship.
--
-- If overriding this function in a subclass you should be careful to ensure
-- the parent class's implementation is always called.
--
-- Parameters:
--
--  ship - Ship, the ship this equipment item is being installed in.
--
--  slot - optional HullConfig.Slot, the slot this item is being installed in
--         if it is a slot-mounted equipment item.
--
---@param ship Ship
---@param slot HullConfig.Slot?
function EquipType:OnInstall(ship, slot)
	-- Extend this for any custom installation logic needed
	-- (e.g. mounting weapons)

	-- Create unique instances of the slots provided by this equipment item
	if self.provides_slots and not rawget(self, "provides_slots") then
		self.provides_slots = utils.map_table(self.provides_slots, function(id, slot) return id, slot:clone() end)
	end
end

-- Method: OnRemove
--
-- Perform any setup associated with uninstalling this item from a Ship.
--
-- If overriding this function in a subclass you should be careful to ensure
-- the parent class's implementation is always called.
--
-- Parameters:
--
--  ship - Ship, the ship this equipment item is being removed from.
--
--  slot - optional HullConfig.Slot, the slot this item is being removed from
--         if it is a slot-mounted equipment item.
--
---@param ship Ship
---@param slot HullConfig.Slot?
function EquipType:OnRemove(ship, slot)
	-- Override this for any custom uninstallation logic needed
end

-- Method: isProto
--
-- Returns true if this object is an equipment item prototype, false if it is
-- an instance.
function EquipType:isProto()
	return not rawget(self, "__proto")
end

-- Method: GetPrototype
--
-- Return the prototype this equipment item instance is derived from, or the
-- self argument if called on a prototype directly.
---@return EquipType
function EquipType:GetPrototype()
	return rawget(self, "__proto") or self
end

-- Method: Instance
--
-- Create and return an instance of this equipment prototype.
---@return EquipType
function EquipType:Instance()
	return setmetatable({ __proto = self }, self.meta)
end

-- Method: SetCount
--
-- Update this equipment instance's stats to represent a logical "stack" of the
-- same item. This should never be called on an instance that is already
-- installed in an EquipSet.
--
-- Some equipment slots represent multiple in-world items as a single logical
-- "item" for the player to interact with. This function handles scaling
-- equipment stats according to the number of "copies" of the item this
-- instance represents.
---@param count integer
function EquipType:SetCount(count)
	local proto = self:GetPrototype()

	self.mass = proto.mass * count
	self.volume = proto.volume * count
	self.price = proto.price * count
	self.count = count
end

-- Patch an EquipType class to support a prototype-based equipment system
-- `equipProto = EquipType.New({ ... })` to create an equipment prototype
-- `equipInst = equipProto:Instance()` to create a new instance based on the created prototype
function EquipType.SetupPrototype(type)
	local old = type.New
	local un = type.Unserialize

	-- Create a new metatable for instances of the prototype object;
	-- delegates serialization to the base class of the proto
	function type.New(...)
		local inst = old(...)
		inst.meta = utils.mixin(type.meta, { __index = inst })
		return inst
	end

	function type.Unserialize(inst)
		inst = un(inst) ---@type any

		-- if we have a "__proto" field we're an instance of the equipment prototype
		if rawget(inst, "__proto") then
			setmetatable(inst, inst.__proto.meta)
		end

		return inst
	end
end

function EquipType:Serialize()
	local tmp = EquipType.Super().Serialize(self)
	local ret = {}
	for k,v in pairs(tmp) do
		if type(v) ~= "function" then
			ret[k] = v
		end
	end

	ret.transient = nil
	return ret
end

function EquipType.Unserialize(data)
	local obj = EquipType.Super().Unserialize(data)
	setmetatable(obj, EquipType.meta)

	-- Only patch the common prototype with runtime transient data
	if EquipType.isProto(obj) then
		EquipType._createTransient(obj)
	end

	return obj
end

-- Method: GetName
--
-- Returns the translated name of this equipment item suitable for display to
-- the user.
---@return string
function EquipType:GetName()
	return self.transient.name
end

-- Method: GetDescription
--
-- Returns the translated description of this equipment item suitable for
-- display to the user
---@return string
function EquipType:GetDescription()
	return self.transient.description
end

--==============================================================================

-- Base type for weapons
---@class Equipment.LaserType : EquipType
---@field laser_stats table
local LaserType = utils.inherits(EquipType, "Equipment.LaserType")

---@param ship Ship
---@param slot HullConfig.Slot
function LaserType:OnInstall(ship, slot)
	EquipType.OnInstall(self, ship, slot)

	for k, v in pairs(self.laser_stats) do
		-- TODO: allow installing more than one laser
		ship:setprop('laser_front_' .. k, v)
	end
end

---@param ship Ship
---@param slot HullConfig.Slot
function LaserType:OnRemove(ship, slot)
	EquipType.OnRemove(self, ship, slot)

	for k, v in pairs(self.laser_stats) do
		-- TODO: allow installing more than one laser
		ship:setprop('laser_front_' .. k, nil)
	end
end

--==============================================================================

-- Single drive type, no support for slave drives.
---@class Equipment.HyperdriveType : EquipType
---@field fuel CommodityType
---@field byproduct CommodityType?
local HyperdriveType = utils.inherits(EquipType, "Equipment.HyperdriveType")

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
	if ship:GetInstalledHyperdrive() ~= self or source:IsSameSystem(destination) then
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
	-- NOTE: this enforces the constraint that only one hyperdrive may be installed on a ship
	local engine = ship:GetInstalledHyperdrive()
	if engine ~= self then
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

--==============================================================================

-- NOTE: "sensors" have no general-purpose code associated with the equipment type
---@class Equipment.SensorType : EquipType
local SensorType = utils.inherits(EquipType, "Equipment.SensorType")

--==============================================================================

-- NOTE: all code related to managing a body scanner is implemented in the ScanManager component
---@class Equipment.BodyScannerType : EquipType
---@field stats table
local BodyScannerType = utils.inherits(SensorType, "Equipment.BodyScannerType")

--==============================================================================

---@class Equipment.CabinType : EquipType
---@field passengers Character[]?
local CabinType = utils.inherits(EquipType, "Equipment.CabinType")

---@param passenger Character
function CabinType:AddPassenger(passenger)
	table.insert(self.passengers, passenger)
	self.icon_name = "equip_cabin_occupied"
end

---@param passenger Character
function CabinType:RemovePassenger(passenger)
	utils.remove_elem(self.passengers, passenger)
	self.icon_name = "equip_cabin_empty"
end

function CabinType:HasPassenger(passenger)
	return utils.contains(self.passengers, passenger)
end

function CabinType:GetNumPassengers()
	return self.passengers and #self.passengers or 0
end

function CabinType:GetMaxPassengers()
	return self.capabilities.cabin
end

function CabinType:GetFreeBerths()
	return self.capabilities.cabin - (self.passengers and #self.passengers or 0)
end

function CabinType:OnInstall(ship, slot)
	EquipType.OnInstall(self, ship, slot)

	self.passengers = {}
end

function CabinType:OnRemove(ship, slot)
	EquipType.OnRemove(self, ship, slot)

	if self.passengers then
		logWarning("Removing passenger cabin with passengers onboard!")
		ship:setprop("cabin_occupied_cap", ship["cabin_occupied_cap"] - #self.passengers)
	end
end

--==============================================================================

---@class Equipment.ThrusterType : EquipType
local ThrusterType = utils.inherits(EquipType, "Equipment.ThrusterType")

--==============================================================================

Serializer:RegisterClass("EquipType", EquipType)
Serializer:RegisterClass("Equipment.LaserType", LaserType)
Serializer:RegisterClass("Equipment.HyperdriveType", HyperdriveType)
Serializer:RegisterClass("Equipment.SensorType", SensorType)
Serializer:RegisterClass("Equipment.BodyScannerType", BodyScannerType)
Serializer:RegisterClass("Equipment.CabinType", CabinType)
Serializer:RegisterClass("Equipment.ThrusterType", ThrusterType)

EquipType:SetupPrototype()
LaserType:SetupPrototype()
HyperdriveType:SetupPrototype()
SensorType:SetupPrototype()
BodyScannerType:SetupPrototype()
CabinType:SetupPrototype()
ThrusterType:SetupPrototype()

return {
	laser			= laser,
	hyperspace		= hyperspace,
	misc			= misc,
	EquipType		= EquipType,
	LaserType		= LaserType,
	HyperdriveType	= HyperdriveType,
	SensorType		= SensorType,
	BodyScannerType	= BodyScannerType,
	CabinType       = CabinType,
	ThrusterType    = ThrusterType,
}
