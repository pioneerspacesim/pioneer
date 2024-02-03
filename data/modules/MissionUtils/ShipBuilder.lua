-- Copyright © 2008-2023 Pioneer Developers. See AUTHORS.txt for details
-- Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

local Engine     = require 'Engine'
local Equipment  = require 'Equipment'
local EquipSet   = require 'EquipSet'
local ShipDef    = require 'ShipDef'
local ShipConfig = require 'ShipConfig'
local Space      = require 'Space'
local Ship       = require 'Ship'

local utils = require 'utils'

local slotTypeMatches = function(slot, filter)
	return string.sub(slot, 1, #filter) == filter
end

-- Class: MissionUtils.ShipBuilder
--
-- Utilities for spawning and equipping NPC ships to be used in mission modules

---@class MissionUtils.ShipBuilder
local ShipBuilder = {}

local hyperdriveRule = {
	slot = "hyperdrive",
}

local randomPulsecannonEasyRule = {
	slot = "weapon",
	filter = "weapon.energy.pulsecannon",
	pick = "random",
	maxSize = 3,
	limit = 1
}

local atmoShieldRule = {
	slot = "hull",
	filter = "hull.atmo_shield",
	limit = 1
}

local pirateProduction = {
	role = "pirate",
	rules = {
		hyperdriveRule,
		randomPulsecannonEasyRule,
		atmoShieldRule,
		{
			slot = "shield",
			limit = 1
		},
		{
			slot = "computer",
			equip = "misc.autopilot",
			limit = 1
		}
	}
}

---@param shipPlan table
---@param shipConfig ShipDef.Config
---@param rule table
---@param rand Rand
function ShipBuilder.ApplyEquipmentRule(shipPlan, shipConfig, rule, rand)

	local addEquipToPlan = function(equip, slot)
		shipPlan.slots[slot.id] = equip
		shipPlan.freeVolume = shipPlan.freeVolume - equip.volume
		shipPlan.equipMass = shipPlan.equipMass + equip.volume
	end

	local matchRuleSlot = function(slot)
		return slotTypeMatches(slot.type, rule.slot)
			and (not rule.maxSize or slot.size <= rule.maxSize)
			and (not rule.minSize or slot.size >= rule.minSize)
	end

	-- Get a list of all equipment slots on the ship that match this rule
	local slots = utils.to_array(shipConfig.slots, function(slot)
		-- Don't install in already-filled slots
		return not shipPlan.slots[slot.id]
			and matchRuleSlot(slot)
	end)

	-- Early-out if we have nowhere to install equipment
	if #slots == 0 then return end

	-- Sort the table of slots so we install in the best/biggest slot first
	table.sort(slots, function(a, b) return a.size > b.size or (a.size == b.size and a.id < b.id) end)

	-- Track how many items have been installed total
	local numInstalled = 0

	-- Explicitly-specified equipment item, just add it to slots as able
	if rule.equip then

		local equip = Equipment.Get(rule.equip)

		for _, slot in ipairs(slots) do
			if EquipSet.CompatibleWithSlot(equip, slot) and shipPlan.freeVolume >= equip.volume then
				addEquipToPlan(equip, slot)
			end

			numInstalled = numInstalled + 1

			if rule.limit and numInstalled >= rule.limit then
				break
			end
		end

		return
	end

	-- Limit equipment according to what will actually fit the ship
	-- NOTE: this does not guarantee all slots will be able to be filled in a balanced manner
	local maxVolume = rule.balance and shipPlan.freeVolume / #slots or shipPlan.freeVolume

	-- Build a list of all equipment items that could potentially be installed
	local filteredEquip = utils.to_array(Equipment.new, function(equip)
		return (equip.slot or false)
			and matchRuleSlot(equip.slot)
			and equip.volume <= maxVolume
			and (not rule.filter or slotTypeMatches(equip.slot.type, rule.filter))
	end)

	-- No equipment items can be installed, rule is finished
	if #filteredEquip == 0 then
		return
	end

	-- Iterate over each slot and install items
	for _, slot in ipairs(slots) do

		-- Not all equipment items which passed the size/slot type check earlier
		-- may be compatible with this specific slot (e.g. if it has a more
		-- specific slot type than the rule itself).
		local compatible = utils.filter_array(filteredEquip, function(equip)
			return EquipSet.CompatibleWithSlot(equip, slot)
				and shipPlan.freeVolume >= equip.volume
		end)

		-- Nothing fits in this slot, ignore it then
		if #compatible > 0 then

			if rule.pick == "random" then
				-- Select a random item from the list
				local equip = compatible[rand:Integer(1, #compatible)]
				addEquipToPlan(equip, slot)
			else
				-- Sort equipment items by size; heavier items of the same size
				-- class first. Assume the largest and heaviest item is the best,
				-- since we don't have any markup data to tell otherwise
				table.sort(compatible, function(a, b)
					return a.slot.size > b.slot.size or (a.slot.size == b.slot.size and a.mass > b.mass)
				end)

				addEquipToPlan(compatible[1], slot)
			end

			numInstalled = numInstalled + 1

			if rule.limit and numInstalled > rule.limit then
				break
			end

		end

	end

end

function ShipBuilder.SelectHull(production)
	local hullList = {}

	-- TODO: some sort of balance metric should be expressed here

	for id, shipDef in pairs(ShipDef) do
		if utils.contains(shipDef.roles, production.role) then
			table.insert(hullList, id)
		end
	end

	if #hullList == 0 then
		return nil
	end

	local selectedHull = hullList[Engine.rand:Integer(1, #hullList)]

	return ShipConfig[selectedHull]
end

---@param production table
---@param shipConfig ShipDef.Config
function ShipBuilder.MakePlan(production, shipConfig)

	local shipPlan = {
		shipId = shipConfig.id,
		equipMass = 0,
		freeVolume = shipConfig.capacity,
		slots = {}
	}

	for _, rule in ipairs(production.rules) do
		ShipBuilder.ApplyEquipmentRule(shipPlan, shipConfig, rule, Engine.rand)
	end

	return shipPlan

end

---@param ship Ship
---@param shipPlan table
function ShipBuilder.ApplyPlan(ship, shipPlan)

	local equipSet = ship:GetComponent('EquipSet')

	for name, proto in pairs(shipPlan.slots) do
		local slot = equipSet:GetSlotHandle(name)
		assert(slot)

		equipSet:Install(proto(), slot)
	end

	-- TODO: loose equipment

end

---@param player Ship
---@param nearDist number?
---@param farDist number?
function ShipBuilder.MakeGenericPirateNear(player, risk, nearDist, farDist)

	local hullConfig = ShipBuilder.SelectHull(pirateProduction)
	assert(hullConfig)

	local plan = ShipBuilder.MakePlan(pirateProduction, hullConfig)
	assert(plan)

	---@type Ship
	local ship = Space.SpawnShipNear(plan.shipId, player, nearDist or 50, farDist or 100)
	ship:SetLabel(Ship.MakeRandomLabel())

	ShipBuilder.ApplyPlan(ship, plan)

	-- TODO: handle risk/difficulty-based installation of equipment as part of
	-- the loadout rules
	local equipSet = ship:GetComponent('EquipSet')

	if Engine.rand:Number(2) <= risk then
		equipSet:Install(Equipment.Get("misc.laser_cooling_booster"))
	end

	if Engine.rand:Number(3) <= risk then
		equipSet:Install(Equipment.Get("misc.shield_energy_booster"))
	end

	return ship

end

require 'Event'.Register("onEnterMainMenu", function()
	local plan = ShipBuilder.MakePlan(pirateProduction, ShipConfig['coronatrix'])

	utils.print_r(plan)
end)

return ShipBuilder
