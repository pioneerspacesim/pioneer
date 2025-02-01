-- Copyright © 2008-2025 Pioneer Developers. See AUTHORS.txt for details
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
	-- Create the instance metatable for instances of this prototype
	-- NOTE: intentionally duplicated in EquipType.NewType
	obj.meta = utils.mixin(EquipType.meta, { __index = obj })

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
		flavourtext = l:get(obj.l10n_key .. "_FLAVOURTEXT") or "",
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

-- Method: isInstance
--
-- Returns true if this object is an equipment item instance, false if it is
-- n prototype.
function EquipType:isInstance()
	return rawget(self, "__proto") ~= nil
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
	assert(self:isInstance())
	local proto = self:GetPrototype()

	self.mass = proto.mass * count
	self.volume = proto.volume * count
	self.price = proto.price * count
	self.count = count
end

-- Function: NewType
--
-- Create a new type of equipment inheriting from the given base type
-- (typically passed as the self argument)
--
-- This function is responsible for overriding the .New() function to create
-- serializable equipment object prototypes. To set values at creation time
-- on prototypes created from this type, you should override the Constructor()
-- method.
--
-- --- Lua ---
-- -- Create a new equipment type
-- MyType = EquipType:NewType("Equipment.MyType")
-- -- Create an equipment item prototype
-- equipProto = MyType.New({ ... })
-- -- Create a new equipment item instance from that prototype
-- equipInst = equipProto:Instance()
-- -----------
function EquipType.NewType(baseType, name)
	local newType = utils.class(name, baseType)

	function newType.New(...)
		-- Create a prototype object and set its metatable appropriately
		local proto = setmetatable(baseType.New(...), newType.meta)
		-- Create the instance metatable for instances of this prototype;
		-- delegates serialization to the base class of the prototype
		proto.meta = utils.mixin(newType.meta, { __index = proto })
		-- Run the constructor for this type
		newType.Constructor(proto, ...)

		return proto
	end

	-- Override the unserializer to handle prototype metatables
	-- If the new type defines an Unserialize method it must store this unserializer method and call it
	function newType.Unserialize(data)
		local inst = baseType.Unserialize(data)
		local proto = rawget(inst, "__proto")
		return setmetatable(inst, proto and proto.meta or newType.meta)
	end

	Serializer:RegisterClass(name, newType)

	return newType
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
	else
		-- This equipment instance's prototype should be set as the metatable
		setmetatable(obj, obj.__proto.meta)
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

-- Method: GetFlavourText
--
-- Returns the translated tooltip for this equipment item suitable for
-- display to the user
---@return string
function EquipType:GetFlavourText()
	return self.transient.flavourtext
end

Serializer:RegisterClass("EquipType", EquipType)

return EquipType
