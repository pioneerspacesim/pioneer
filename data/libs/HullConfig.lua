-- Copyright © 2008-2026 Pioneer Developers. See AUTHORS.txt for details
-- Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

local ShipDef = require 'ShipDef'
local Serializer = require 'Serializer'
local Equipment = require 'Equipment'
require 'modules.Equipment.Internal'

local utils = require 'utils'

-- Class: HullConfig.Slot
--
-- Generic interface for an equipment-containing slot in a shipdef
--
-- Represents a constrained potential mounting point for ship equipment
-- either internal or external to a ship.
-- Can contain additional fields for specific slot types.
---@class HullConfig.Slot
---@field clone fun(self, mixin):self
local Slot = utils.proto("HullConfig.Slot")

Slot.id = ""
Slot.type = ""
Slot.size = 1
Slot.size_min = nil ---@type number?
Slot.tag = nil ---@type string?
Slot.default = nil ---@type string?
Slot.required = false ---@type boolean
Slot.hardpoint = false
Slot.i18n_key = nil ---@type string?
Slot.i18n_res = "equipment-core"
Slot.count = nil ---@type integer?
Slot.gimbal = nil ---@type table?

-- Class: HullConfig
--
-- Represents a specific "ship configuration", being a list of equipment slots
-- and associated data.
--
-- The default configuration for a ship hull is defined in its JSON shipdef and
-- consumed by Lua as a ship config.
---@class HullConfig
---@field clone fun():self
local HullConfig = utils.proto("HullConfig")

HullConfig.id = ""
HullConfig.path = ""
HullConfig.equipCapacity = 0
HullConfig.maxPayload = 0

-- Default slot config for a new shipdef
-- Individual shipdefs can redefine slots or remove them by setting the slot to 'false'
---@type table<string, HullConfig.Slot>
HullConfig.slots = {
	sensor = Slot:clone { type = "sensor", size = 1 },
}

function HullConfig:__clone()
	self.slots = utils.map_table(self.slots, function(key, slot)
		return key, slot:clone()
	end)
end

Serializer:RegisterClass("HullConfig", HullConfig)
Serializer:RegisterClass("HullConfig.Slot", Slot)

--==============================================================================

-- Function: AddAutoCabinSlots
--
-- Instead of meticulously filling out all ship definitions with cabin slots
-- add them automatically according to the payload capacity of the ship.
local function AddAutoCabinSlots(newShip, def)

	-- Remove any existing cabin slots that don't have unique keys.
	for name, slot in pairs(newShip.slots) do
		if slot.type == "cabin" then
			if slot.i18n_key == nul then
				newShip.slots[name] = nil
			end
		end
	end

	local space = def.cargo or 0
	local biggest = 0

	-- Start with the largest possible cabin size and work down
	for size = 4, 1, -1 do

		local eq = Equipment.new["misc.cabin_s" .. tostring(size)]
		local slot_mass = eq and eq.mass or math.huge

		local slot_target = math.floor(space / slot_mass)

		if size == biggest - 1 then
			-- Whatever the biggest size cabin we can add is, we also want to add
			-- at least one of the next size down, to ensure that the user always
			-- has some extra cabin options to play with
			if slot_target == 0 then
				slot_target = 1
			end
		end

		local slot_count = 0
		for _, slot in pairs(newShip.slots) do
			if slot.type == "cabin" and slot.size == size then
				slot_count = slot_count + 1
			end
		end

		local slot_need = math.max(0, slot_target - slot_count)
		for i = 1, slot_need do
			local name = string.format("auto_cabin_s%d_%02d", size, i)
			newShip.slots[name] = Slot:clone {
				id = name,
				type = "cabin",
				size = size,
				size_min = 1,
				i18n_key = "SLOT_CABIN",
			}
		end

		if slot_target > 0 and biggest == 0 then
			biggest = size
		end

		space = space - (slot_mass * slot_target)
	end
end

local function CreateShipConfig(def)
	local newShip = HullConfig:clone()
	Serializer:RegisterPersistent("ShipDef." .. def.id, newShip)

	newShip.id = def.id
	newShip.path = def.path
	newShip.equipCapacity = def.equipCapacity
	newShip.maxPayload = def.cargo

	table.merge(newShip.slots, def.raw.equipment_slots or {}, function(name, slotDef)
		if slotDef == false then return name, nil end

		local newSlot = table.merge(Slot:clone(), slotDef)

		return name, newSlot
	end)

	for name, slot in pairs(newShip.slots) do
		slot.id = name
	end

	AddAutoCabinSlots(newShip, def)

	return newShip
end

---@type table<string, HullConfig>
local Configs = {}

for id, def in pairs(ShipDef) do
	if def.tag == "SHIP" or def.tag == "STATIC_SHIP" then
		Configs[id] = CreateShipConfig(def)
	end
end

-- Function: GetHullConfigs
--
-- Return a table containing all registered hull configurations.
---@return table<string, HullConfig>
local function GetHullConfigs()
	return Configs
end

-- Function: GetHullConfig
--
-- Return the hull configuration corresponding to the given ID
--
---@param id string
---@return HullConfig
local function GetHullConfig(id)
	return Configs[id]
end

--==============================================================================

return {
	Config = HullConfig,
	Slot = Slot,
	GetHullConfigs = GetHullConfigs,
	GetHullConfig = GetHullConfig
}
