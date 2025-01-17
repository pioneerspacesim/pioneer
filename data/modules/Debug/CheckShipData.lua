-- Copyright © 2008-2025 Pioneer Developers. See AUTHORS.txt for details
-- Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

local Equipment  = require 'Equipment'
local HullConfig = require 'HullConfig'
local Loader     = require '.DebugLoader'
local EquipSet   = require 'EquipSet'
local Lang       = require 'Lang'
local ShipDef    = require 'ShipDef'

local utils      = require 'utils'

-- This file implements validation passes for ship JSON files
-- It's intended to catch most common errors, especially those that would be
-- difficult to find outside of switching to each ship type in sequence.

local activeFile = nil

local error = function(message) Loader.LogFileMessage(Loader.Type.Error, activeFile, message) end
local warn = function(message) Loader.LogFileMessage(Loader.Type.Warn, activeFile, message) end
local info = function(message) Loader.LogFileMessage(Loader.Type.Info, activeFile, message) end

local function findMatchingSlots(config, type)
	return utils.filter_table(config.slots, function(_, slot)
		return EquipSet.SlotTypeMatches(slot.type, type)
	end)
end

---@param slot HullConfig.Slot
local function checkSlot(slot)

	if string.match(slot.id, "##") then
		error("Slot {id} name contains invalid sequence '##'." % slot)
	end

	if not string.match(slot.id, "^[a-zA-Z0-9_]+$") then
		warn("Slot {id} name contains non-identifier characters." % slot)
	end

	if slot.required and not slot.default then
		error("Slot {id} is a required slot but does not have a default equipment item." % slot)
	end

	if slot.default and not Equipment.Get(slot.default) then
		error("Slot {id} default item ({default}) does not exist." % slot)
	end

	if EquipSet.SlotTypeMatches(slot.type, "hyperdrive") and not slot.default then
		info("Slot {id} has no default hyperdrive equipment and will not have a hyperdrive when purchased." % slot)
	end

	if slot.i18n_key then
		if not slot.i18n_res then
			error("Slot {id} has an invalid language resource key {i18n_res}." % slot)
		end

		local res = Lang.GetResource(slot.i18n_res)

		if not rawget(res, slot.i18n_key) then
			warn("Slot {id} uses undefined lang string '{i18n_res}.{i18n_key}'." % slot)
		end
	end

	local isWeaponType = EquipSet.SlotTypeMatches(slot.type, "weapon")
	local isPylonType = EquipSet.SlotTypeMatches(slot.type, "pylon")
	local isBayType = EquipSet.SlotTypeMatches(slot.type, "missile_bay")
	local isScoopType = EquipSet.SlotTypeMatches(slot.type, "fuel_scoop")

	local isExternal = isWeaponType or isPylonType or isBayType or isScoopType

	if isExternal then

		if not slot.hardpoint then
			error("External slot {id} with type {type} should have hardpoint=true." % slot)
		end

		if not slot.tag then
			warn("External slot {id} with type {type} is missing an associated tag." % slot)
		end

	end

	if isWeaponType then

		if not slot.gimbal or type(slot.gimbal) ~= "table" then
			error("Weapon slot {id} is missing gimbal data." % slot)
		elseif type(slot.gimbal[1]) ~= "number" or type(slot.gimbal[2]) ~= "number" then
			error("Weapon slot {id} should have a two-axis gimbal expressed as [x, y]." % slot)
		end

	end

end

---@param config HullConfig
local function checkConfig(config)
	if utils.count(findMatchingSlots(config, "hyperdrive")) > 1 then
		error("Ship {id} has more than one hyperdrive slot; this will break module code." % config)
	end

	if utils.count(findMatchingSlots(config, "thruster")) == 0 then
		warn("Ship {id} has no thruster slots. This may break in the future.")
	end

	if utils.count(findMatchingSlots(config, "hull")) == 0 then
		info("Ship {id} has no hull modification slots." % config)
	end

	if utils.count(findMatchingSlots(config, "structure")) == 0 then
		info("Ship {id} has no structural modification slots." % config)
	end

	if utils.count(findMatchingSlots(config, "computer")) == 0 then
		info("Ship {id} has no computer equipment slots." % config)
	end

	if utils.count(findMatchingSlots(config, "sensor")) == 0 then
		info("Ship {id} has no sensor slots." % config)
	end

	-- TODO: more validation passes on the whole ship config
end

---@param shipDef ShipDef
local function checkShipDef(shipDef)
	if shipDef.tag ~= "SHIP" then
		return
	end

	if utils.count(shipDef.roles) == 0 then
		info("Ship {id} has no roles and will not be used by most modules." % shipDef)
	end

	if shipDef.minCrew > shipDef.maxCrew then
		error("Ship {id} has minCrew {minCrew} > maxCrew {maxCrew}." % shipDef)
	end

	if not shipDef.shipClass or shipDef.shipClass == "" then
		warn("Ship {id} has missing/empty ship_class field." % shipDef)
	end

	if not shipDef.manufacturer then
		info("Ship {id} has no manufacturer set." % shipDef)
	end
end

Loader.RegisterCheck("HullConfigs", function()

	local configs = HullConfig.GetHullConfigs()

	for _, config in pairs(configs) do
		activeFile = config.path

		for _, slot in pairs(config.slots) do checkSlot(slot) end
		checkConfig(config)
	end

	for _, shipDef in pairs(ShipDef) do
		activeFile = shipDef.path

		checkShipDef(shipDef)
	end

end)
