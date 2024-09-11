-- Copyright © 2008-2023 Pioneer Developers. See AUTHORS.txt for details
-- Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

local Engine     = require 'Engine'
local Equipment  = require 'Equipment'
local EquipSet   = require 'EquipSet'
local ShipDef    = require 'ShipDef'
local ShipConfig = require 'ShipConfig'
local Space      = require 'Space'
local Ship       = require 'Ship'

local Rules = require '.OutfitRules'

local utils = require 'utils'

local hullThreatCache = {}

local slotTypeMatches = function(slot, filter)
	return string.sub(slot, 1, #filter) == filter
end

-- Class: MissionUtils.ShipBuilder
--
-- Utilities for spawning and equipping NPC ships to be used in mission modules

---@class MissionUtils.ShipBuilder
local ShipBuilder = {}

-- =============================================================================

-- Scalars determining a "threat factor" rating for a ship's hull and equipment
-- to generate difficulty-appropriate NPCs

-- Default values to use if a mission hasn't set anything
ShipBuilder.kDefaultRandomThreatMin = 10.0
ShipBuilder.kDefaultRandomThreatMax = 100.0

-- || Hull Threat Factor ||

-- A ship hull starts with at least this much threat
ShipBuilder.kBaseHullThreatFactor = 1.0

-- How much hull health contributes to ship threat factor (tons of armor to threat)
ShipBuilder.kArmorToThreatFactor = 0.15

-- How much the ship's maximum forward acceleration contributes to threat factor
ShipBuilder.kAccelToThreat = 0.000015
-- Tbreat from acceleration is added to this number to determine the final modifier for ship hull HP
ShipBuilder.kAccelThreatBase = 0.5

-- Controls how a ship's atmospheric performance contributes to its threat factor
ShipBuilder.kAeroStabilityToThreat = 0.3
ShipBuilder.kAeroStabilityThreatBase = 0.80

ShipBuilder.kCrossSectionToThreat = 0.3
ShipBuilder.kCrossSectionThreatBase = 0.8

-- Only accept ships where the hull is at least this fraction of the desired total threat factor
ShipBuilder.kMinHullThreatFactor = 0.4
ShipBuilder.kMaxHullThreatFactor = 0.8
ShipBuilder.kMinReservedEquipThreat = 4.0

-- || Weapon Threat Factor ||

-- Damage in unit tons of armor per second to threat factor
ShipBuilder.kWeaponDPSToThreat = 0.35

-- Weapon projectile speed modifies threat, calculated as 1.0 + (speed / 1km/s) * mod
ShipBuilder.kWeaponSpeedToThreat = 0.2

-- Amount of hull threat included in this weapon's threat per unit of weapon damage
ShipBuilder.kWeaponSharedHullThreat = 0.02

-- || Shield Threat Factor ||

-- Threat factor per unit armor ton equivalent of shield health
-- (compare with hull threat factor per unit ton)
ShipBuilder.kShieldThreatFactor = 0.2

-- Portion of hull threat factor to contributed per unit ton
ShipBuilder.kShieldSharedHullThreat = 0.005

-- =============================================================================

local Template = utils.proto("MissionUtils.ShipTemplate")

Template.role = nil
Template.shipId = nil
Template.label = nil
Template.rules = {}

function Template:__clone()
	self.rules = table.copy(self.rules)
end

ShipBuilder.Template = Template

-- =============================================================================

local ShipPlan = utils.proto("MissionUtils.ShipPlan")

ShipPlan.shipId = ""
ShipPlan.label = ""
ShipPlan.freeVolume = 0
ShipPlan.equipMass = 0
ShipPlan.threat = 0
ShipPlan.freeThreat = 0
ShipPlan.slots = {}
ShipPlan.equip = {}

function ShipPlan:__clone()
	self.slots = {}
	self.equip = {}
end

function ShipPlan:AddEquipToPlan(equip, slot, threat)
	if slot then
		self.slots[slot.id] = equip
	else
		table.insert(self.equip, equip)
	end

	self.freeVolume = self.freeVolume - equip.volume
	self.equipMass = self.equipMass + equip.mass
	self.threat = self.threat + (threat or 0)
	self.freeThreat = self.freeThreat - (threat or 0)
end

-- =============================================================================

local function calcWeaponThreat(equip, hullThreat)
	local damage = equip.laser_stats.damage / 1000 -- unit tons of armor
	local speed = equip.laser_stats.speed / 1000

	local dps = damage / equip.laser_stats.rechargeTime
	local speedMod = 1.0

	-- Beam lasers don't factor in projectile speed (instant)
	-- TODO: they should have a separate threat factor instead
	if not equip.laser_stats.beam or equip.laser_stats.beam == 0.0 then
		speedMod = 1.0 + ShipBuilder.kWeaponSpeedToThreat * speed
	end

	local threat = ShipBuilder.kWeaponDPSToThreat * dps * speedMod
		+ ShipBuilder.kWeaponSharedHullThreat * damage * hullThreat

	return threat
end

local function calcShieldThreat(equip, hullThreat)
	-- XXX: this is a hardcoded constant shared with Ship.cpp
	local shield_tons = equip.capabilities.shield * 10.0

	local threat = ShipBuilder.kShieldThreatFactor * shield_tons
		+ ShipBuilder.kShieldSharedHullThreat * shield_tons * hullThreat

	return threat
end

function ShipBuilder.ComputeEquipThreatFactor(equip, hullThreat)
	if equip.slot and equip.slot.type:match("weapon.") and equip.laser_stats then
		return calcWeaponThreat(equip, hullThreat)
	end

	if equip.slot and equip.slot.type:match("shield.") and equip.capabilities.shield then
		return calcShieldThreat(equip, hullThreat)
	end

	return 0.0
end

-- Function: ComputeHullThreatFactor
--
-- Compute a "balance number" according to a ship's potential combat threat to
-- be used when spawning a random ship for specific encounters.
--
-- This function does not take into account the "potential threat" of a ship if
-- its slots were to be filled (or even the possible configurations of slots),
-- but only looks at concrete stats about the ship hull.
---@param shipDef ShipDef
function ShipBuilder.ComputeHullThreatFactor(shipDef)
	local threat = { id = shipDef.id }

	local armor = shipDef.hullMass
	local totalMass = shipDef.hullMass + shipDef.fuelTankMass
	local forwardAccel = shipDef.linearThrust["FORWARD"] / totalMass
	local crossSectionAvg = (shipDef.topCrossSec + shipDef.sideCrossSec + shipDef.frontCrossSec) / 3.0

	threat.armor = ShipBuilder.kBaseHullThreatFactor + ShipBuilder.kArmorToThreatFactor * armor
	threat.thrust = ShipBuilder.kAccelThreatBase + ShipBuilder.kAccelToThreat * forwardAccel
	threat.aero = ShipBuilder.kAeroStabilityThreatBase + ShipBuilder.kAeroStabilityToThreat * (shipDef.raw.aero_stability or 0.0)
	threat.crosssection = ShipBuilder.kCrossSectionThreatBase + ShipBuilder.kCrossSectionToThreat * (armor / crossSectionAvg)

	threat.total = threat.armor * threat.thrust * threat.aero * threat.crosssection
	threat.total = utils.round(threat.total, 0.01)

	return threat
end

-- =============================================================================

---@param shipPlan table
---@param shipConfig ShipDef.Config
---@param rule table
---@param rand Rand
---@param hullThreat number
function ShipBuilder.ApplyEquipmentRule(shipPlan, shipConfig, rule, rand, hullThreat)

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
		local threat = ShipBuilder.ComputeEquipThreatFactor(equip, hullThreat)

		for _, slot in ipairs(slots) do

			local canInstall = shipPlan.freeVolume >= equip.volume
				and shipPlan.freeThreat >= threat
				and EquipSet.CompatibleWithSlot(equip, slot)

			if canInstall then
				shipPlan:AddEquipToPlan(equip, slot, threat)
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

	-- Build a cache of the threat values for these equipment items
	-- We may have multiple rounds of the install loop, so it's better to do it now
	--
	-- NOTE: if equipment items don't include hull threat in their calculation, this
	-- can be precached at startup
	local threatCache = utils.map_table(filteredEquip, function(_, equip)
		return equip, ShipBuilder.ComputeEquipThreatFactor(equip, hullThreat)
	end)

	-- Iterate over each slot and install items
	for _, slot in ipairs(slots) do

		-- Not all equipment items which passed the size/slot type check earlier
		-- may be compatible with this specific slot (e.g. if it has a more
		-- specific slot type than the rule itself).
		local compatible = utils.filter_array(filteredEquip, function(equip)
			return EquipSet.CompatibleWithSlot(equip, slot)
				and shipPlan.freeVolume >= equip.volume
				and shipPlan.freeThreat >= threatCache[equip]
		end)

		-- Nothing fits in this slot, ignore it then
		if #compatible > 0 then

			if rule.pick == "random" then
				-- Select a random item from the list
				local equip = compatible[rand:Integer(1, #compatible)]
				shipPlan:AddEquipToPlan(equip, slot, threatCache[equip])
			else
				-- Sort equipment items by size; heavier items of the same size
				-- class first. Assume the largest and heaviest item is the best,
				-- since we don't have any markup data to tell otherwise
				table.sort(compatible, function(a, b)
					return a.slot.size > b.slot.size or (a.slot.size == b.slot.size and a.mass > b.mass)
				end)

				-- Just install the "best" item we have
				local equip = compatible[1]
				shipPlan:AddEquipToPlan(equip, slot, threatCache[equip])
			end

			numInstalled = numInstalled + 1

			if rule.limit and numInstalled > rule.limit then
				break
			end

		end

	end

end

-- =============================================================================

function ShipBuilder.GetHullThreat(shipId)
	return hullThreatCache[shipId] or { total = 0.0 }
end

function ShipBuilder.SelectHull(template, threat)
	local hullList = {}

	if template.shipId then

		table.insert(hullList, template.shipType)

	else

		for id, shipDef in pairs(ShipDef) do

			if shipDef.roles[template.role] then

				local hullThreat = ShipBuilder.GetHullThreat(id).total

				-- Use threat metric as a way to balance the random selection of ship hulls
				local withinRange = hullThreat >= ShipBuilder.kMinHullThreatFactor * threat
					and hullThreat <= ShipBuilder.kMaxHullThreatFactor * threat
				local hasReserve = threat - hullThreat >= ShipBuilder.kMinReservedEquipThreat

				if withinRange and hasReserve then
					table.insert(hullList, id)
				end

			end

		end

	end

	if #hullList == 0 then
		return nil
	end

	local shipId = hullList[Engine.rand:Integer(1, #hullList)]

	return ShipConfig[shipId]
end

---@param production table
---@param shipConfig ShipDef.Config
function ShipBuilder.MakePlan(production, shipConfig, threat)

	local hullThreat = ShipBuilder.GetHullThreat(shipConfig.id).total

	local shipPlan = ShipPlan:clone {
		shipId = shipConfig.id,
		freeVolume = shipConfig.capacity,
		threat = hullThreat,
		freeThreat = threat - hullThreat,
	}

	for _, rule in ipairs(production.rules) do

		if rule.slot then
			ShipBuilder.ApplyEquipmentRule(shipPlan, shipConfig, rule, Engine.rand, hullThreat)
		else
			local equip = Equipment.Get(rule.equip)
			assert(equip)

			if shipPlan.freeVolume >= equip.volume then
				shipPlan:AddEquipToPlan(equip)
			end
		end

	end

	shipPlan.label = production.label or Ship.MakeRandomLabel()

	return shipPlan

end

---@param ship Ship
---@param shipPlan table
function ShipBuilder.ApplyPlan(ship, shipPlan)

	local equipSet = ship:GetComponent('EquipSet')

	-- Apply slot-based equipment first
	for name, proto in pairs(shipPlan.slots) do
		local slot = equipSet:GetSlotHandle(name)
		assert(slot)

		equipSet:Install(proto(), slot)
	end

	for _, proto in ipairs(shipPlan.equip) do
		equipSet:Install(proto())
	end

	-- TODO: ammunition / other items inside of instanced equipment

	ship:SetLabel(shipPlan.label)

end

-- =============================================================================

---@param body Body
---@param production table
---@param risk number?
---@param nearDist number?
---@param farDist number?
function ShipBuilder.MakeShipNear(body, production, risk, nearDist, farDist)
	if not risk then
		risk = Engine.rand:Number(ShipBuilder.kDefaultRandomThreatMin, ShipBuilder.kDefaultRandomThreatMax)
	end

	local hullConfig = ShipBuilder.SelectHull(production, risk)
	assert(hullConfig)

	local plan = ShipBuilder.MakePlan(production, hullConfig, risk)
	assert(plan)

	local ship = Space.SpawnShipNear(plan.shipId, body, nearDist or 50, farDist or 100)
	assert(ship)

	ShipBuilder.ApplyPlan(ship, plan)

	return ship
end

---@param body Body
---@param production table
---@param risk number?
function ShipBuilder.MakeShipDocked(body, production, risk)
	if not risk then
		risk = Engine.rand:Number(ShipBuilder.kDefaultRandomThreatMin, ShipBuilder.kDefaultRandomThreatMax)
	end

	local hullConfig = ShipBuilder.SelectHull(production, risk)
	assert(hullConfig)

	local plan = ShipBuilder.MakePlan(production, hullConfig, risk)
	assert(plan)

	local ship = Space.SpawnShipDocked(plan.shipId, body)
	assert(ship)

	ShipBuilder.ApplyPlan(ship, plan)

	return ship
end

-- =============================================================================

local randomPulsecannonEasyRule = {
	slot = "weapon",
	filter = "weapon.energy.pulsecannon",
	pick = "random",
	maxSize = 3,
	limit = 1
}

local pirateProduction = Template:clone {
	role = "pirate",
	rules = {
		Rules.DefaultHyperdrive,
		randomPulsecannonEasyRule,
		Rules.DefaultAtmoShield,
		Rules.DefaultShieldGen,
		Rules.DefaultAutopilot
	}
}

---@param player Ship
---@param nearDist number?
---@param farDist number?
function ShipBuilder.MakeGenericPirateNear(player, risk, nearDist, farDist)

	local ship = ShipBuilder.MakeShipNear(player, pirateProduction, risk, nearDist, farDist)

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

-- Generate a cache of combined hull threat factor for each ship in the game
function ShipBuilder.BuildHullThreatCache()
	for id, shipDef in pairs(ShipDef) do
		local threat = ShipBuilder.ComputeHullThreatFactor(shipDef)

		hullThreatCache[id] = threat
	end
end

require 'Event'.Register("onEnterMainMenu", function()
	ShipBuilder.BuildHullThreatCache()

	local threat = 20.0

	local hull = ShipBuilder.SelectHull({ role = "pirate" }, threat)

	local plan = ShipBuilder.MakePlan(pirateProduction, ShipConfig['coronatrix'], threat)

	utils.print_r(plan)
end)

return ShipBuilder
