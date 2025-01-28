-- Copyright © 2008-2025 Pioneer Developers. See AUTHORS.txt for details
-- Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

local Engine     = require 'Engine'
local Equipment  = require 'Equipment'
local EquipSet   = require 'EquipSet'
local Event      = require 'Event'
local HullConfig = require 'HullConfig'
local ShipDef    = require 'ShipDef'
local Space      = require 'Space'
local Ship       = require 'Ship'

local Rules = require '.OutfitRules'

local utils = require 'utils'

local hullThreatCache = {}

local slotTypeMatches = EquipSet.SlotTypeMatches

-- Class: MissionUtils.ShipBuilder
--
-- Utilities for spawning and equipping NPC ships to be used in mission modules.
--
-- This class provides a complete API for describing in generic terms how a ship
-- should be equipped, allows tailoring that equipment for a wanted difficulty
-- of a combat encounter, and hides most of the complexity of dealing with
-- recursive, complicated slot layouts across all ship hulls in the game.
--
-- The function of the ShipBuilder is to enable mission modules to be able to
-- write a single description specifying how a generic ship should be equipped,
-- and apply that description across a wildly varying selection of ships with
-- no commonality in slot IDs or internal layout.
--
-- It also provides limited support for producing "natural" looking loadouts
-- with semi-balanced equipment across multiple slots of the same type.
--
---@class MissionUtils.ShipBuilder
local ShipBuilder = {}

ShipBuilder.OutfitRules = Rules

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
ShipBuilder.kAccelToThreat = 0.02
-- Tbreat from acceleration is added to this number to determine the final modifier for ship hull HP
ShipBuilder.kAccelThreatBase = 0.5

-- Controls how a ship's atmospheric performance contributes to its threat factor
ShipBuilder.kAeroStabilityToThreat = 0.20
ShipBuilder.kAeroStabilityThreatBase = 0.80

ShipBuilder.kCrossSectionToThreat = 1.0
ShipBuilder.kCrossSectionThreatBase = 0.75

-- Only accept ships where the hull is at most this fraction of the desired total threat factor
ShipBuilder.kMaxHullThreatFactor = 0.8
ShipBuilder.kMinHullThreatFactor = 0.12

-- || Weapon Threat Factor ||

-- Damage in unit tons of armor per second to threat factor
ShipBuilder.kWeaponDPSToThreat = 0.35

-- Weapon projectile speed modifies threat, calculated as 1.0 + (speed / 1km/s) * mod
ShipBuilder.kWeaponSpeedToThreat = 0.2

-- Threat factor increase for dual-fire weapons
ShipBuilder.kDualFireThreatFactor = 1.2

-- Threat factor increase for beam lasers
ShipBuilder.kBeamLaserThreatFactor = 1.8

-- Amount of hull threat included in this weapon's threat per unit of weapon damage
ShipBuilder.kWeaponSharedHullThreat = 0.02

-- || Shield Threat Factor ||

-- Threat factor per unit armor ton equivalent of shield health
-- (compare with hull threat factor per unit ton)
ShipBuilder.kShieldThreatFactor = 0.2

-- Portion of hull threat factor to contributed per unit ton
ShipBuilder.kShieldSharedHullThreat = 0.005

-- =============================================================================

-- Class: MissionUtils.ShipTemplate
--
-- This class is the primary data container for mission modules to express how
-- they would like spawned ships to be outfitted.
--
-- Pre-existing templates can be found in MissionUtils.ShipTemplates, and
-- individual mission modules can construct their own specialized templates or
-- clone and modify existing templates.
--
-- Note that the list of equipment rules is shared between clones of a template
-- for performance reasons and should be overwritten in cloned templates with a
-- modified copy as desired.
--
---@class MissionUtils.ShipTemplate
---@field clone fun(self, mixin: { rules: MissionUtils.OutfitRule[] }): self
local Template = utils.proto("MissionUtils.ShipTemplate")

Template.role = nil ---@type string?
Template.hyperclass = nil ---@type number?
Template.shipId = nil ---@type string?
Template.label = nil ---@type string?
Template.randomModifier = nil ---@type number? Scalar modifying the randomChance value of contained equipment rules
Template.rules = {} ---@type MissionUtils.OutfitRule[]

ShipBuilder.Template = Template

-- =============================================================================

-- Internal ship plan class, intended as an opaque object from external users
-- of the API.
---@class MissionUtils.ShipBuilder.ShipPlan
---@field clone fun(self, mixin: table): self
local ShipPlan = utils.proto("MissionUtils.ShipBuilder.ShipPlan")

ShipPlan.config = nil
ShipPlan.shipId = ""
ShipPlan.label = ""
ShipPlan.freeVolume = 0
ShipPlan.equipMass = 0
ShipPlan.threat = 0
ShipPlan.freeThreat = 0
ShipPlan.filled = {} ---@type table<string, EquipType>
ShipPlan.equip = {} ---@type EquipType[]
ShipPlan.install = {} ---@type string[]
ShipPlan.slots = {} ---@type HullConfig.Slot[]

function ShipPlan:__clone()
	self.filled = {}
	self.default = {}
	self.equip = {}
	self.install = {}
	self.slots = {}
end

function ShipPlan:SortSlots()
	-- Stably sort with largest hardpoints first
	table.sort(self.slots, function(a, b) return a.size > b.size or (a.size == b.size and a.id < b.id) end)
end

-- Set the hull config this plan is going to be applied to.
-- Creates the list of slots to populate with equipment items.
function ShipPlan:SetConfig(shipConfig)
	self.config = shipConfig
	self.shipId = shipConfig.id
	self.freeVolume = shipConfig.equipCapacity

	for _, slot in pairs(shipConfig.slots) do
		table.insert(self.slots, slot)
	end

	self:SortSlots()
end

-- Add extra slots from an equipment item to the list of available slots
function ShipPlan:AddSlots(baseId, slots)
	for _, slot in pairs(slots) do
		local id = baseId .. "##" .. slot.id
		table.insert(self.slots, slot:clone({ id = id }))
	end

	self:SortSlots()
end

-- Add the given equipment object to the plan, making an instance as needed
-- and applying its provided slots to the list of slots present in the plan.
function ShipPlan:AddEquipToPlan(equip, slot, threat)
	-- print("Installing " .. equip:GetName())

	if equip:isProto() then
		equip = equip:Instance()
	end

	if slot then
		self.filled[slot.id] = equip
		table.insert(self.install, slot.id)
	else
		table.insert(self.equip, equip)
	end

	self.freeVolume = self.freeVolume - equip.volume
	self.equipMass = self.equipMass + equip.mass
	self.threat = self.threat + (threat or 0)
	self.freeThreat = self.freeThreat - (threat or 0)

	if equip.provides_slots then
		self:AddSlots(slot.id, equip.provides_slots)
	end
end

-- =============================================================================

-- Class: MissionUtils.ShipBuilder

-- Compute threat factor for weapon equipment
local function calcWeaponThreat(equip, hullThreat)
	local damage = equip.laser_stats.damage / 1000 -- unit tons of armor
	local speed = equip.laser_stats.speed / 1000

	local dps = damage / equip.laser_stats.rechargeTime
	local speedMod = 1.0
	local dualMod = 1.0 + equip.laser_stats.dual * ShipBuilder.kDualFireThreatFactor

	-- Beam lasers don't factor in projectile speed (instant)
	-- Instead they have a separate threat factor
	if equip.laser_stats.beam or equip.laser_stats.beam == 0.0 then
		speedMod = ShipBuilder.kBeamLaserThreatFactor
	else
		speedMod = 1.0 + ShipBuilder.kWeaponSpeedToThreat * speed
	end

	local threat = ShipBuilder.kWeaponDPSToThreat * dps * speedMod * dualMod
		+ ShipBuilder.kWeaponSharedHullThreat * damage * hullThreat

	return threat
end

-- Compute threat factor for shield equipment
local function calcShieldThreat(equip, hullThreat)
	-- FIXME: this is a hardcoded constant shared with Ship.cpp
	local shield_tons = equip.capabilities.shield * 10.0

	local threat = ShipBuilder.kShieldThreatFactor * shield_tons
		+ ShipBuilder.kShieldSharedHullThreat * shield_tons * hullThreat

	return threat
end

-- Function: ComputeEquipThreatFactor
--
-- Compute a threat factor for the given equipment item. The equipment threat
-- factor may be modified by the threat of the hull it is installed on.
function ShipBuilder.ComputeEquipThreatFactor(equip, hullThreat)
	if equip.slot and equip.slot.type:match("^weapon") and equip.laser_stats then
		return calcWeaponThreat(equip, hullThreat)
	end

	if equip.slot and equip.slot.type:match("^shield") and equip.capabilities.shield then
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
	local totalMass = shipDef.hullMass + shipDef.fuelTankMass + shipDef.equipCapacity * 0.5
	local forwardAccel = shipDef.linearThrust["FORWARD"] / (1000.0 * totalMass)
	local crossSectionAvg = (shipDef.topCrossSec + shipDef.sideCrossSec + shipDef.frontCrossSec) / 3.0

	threat.armor = ShipBuilder.kBaseHullThreatFactor + ShipBuilder.kArmorToThreatFactor * armor
	threat.thrust = ShipBuilder.kAccelThreatBase + ShipBuilder.kAccelToThreat * forwardAccel
	threat.aero = ShipBuilder.kAeroStabilityThreatBase + ShipBuilder.kAeroStabilityToThreat * (shipDef.raw.aero_stability or 0.0)
	threat.crosssection = ShipBuilder.kCrossSectionThreatBase + ShipBuilder.kCrossSectionToThreat * (armor^(1/3) / crossSectionAvg^(1/2))

	threat.total = threat.armor * threat.thrust * threat.aero * threat.crosssection
	threat.total = utils.round(threat.total, 0.01)

	return threat
end

-- =============================================================================

-- Function: ApplyEquipmentRule
--
-- Apply the passed equipment rule to an in-progress ship plan, observing the
-- limits expressed in the rule regarding threat, size, and number of items
-- installed.
--
-- Intended as private internal API, and should not be called from outside the
-- ShipBuilder module.
--
---@param shipPlan MissionUtils.ShipBuilder.ShipPlan
---@param rule MissionUtils.OutfitRule
---@param rand Rand
---@param hullThreat number
function ShipBuilder.ApplyEquipmentRule(shipPlan, rule, rand, hullThreat)

	-- print("Applying rule:")
	-- utils.print_r(rule)

	---@type HullConfig
	local shipConfig = shipPlan.config

	local matchRuleSlot = function(slot, filter)
		local minSize = slot.size_min or slot.size
		return slotTypeMatches(slot.type, filter)
			and (not rule.maxSize or minSize <= rule.maxSize)
			and (not rule.minSize or slot.size >= rule.minSize)
	end

	-- Get a list of all equipment slots on the ship that match this rule
	local slots = utils.filter_array(shipPlan.slots, function(slot)
		-- Don't install in already-filled slots
		return not shipPlan.filled[slot.id]
			and matchRuleSlot(slot, rule.slot)
	end)

	-- print("Ship slots: " .. #shipPlan.slots)
	-- print("Filtered slots: " .. #slots)

	-- Early-out if we have nowhere to install equipment
	if #slots == 0 then return end

	-- Track how many items have been installed total
	local numInstalled = 0

	-- Explicitly-specified equipment item, just add it to slots as able
	if rule.equip then

		local equip = Equipment.Get(rule.equip)
		local threat = ShipBuilder.ComputeEquipThreatFactor(equip, hullThreat)

		-- Limit maximum threat consumption of this equipment rule
		local reserveThreat = rule.maxThreatFactor and ((1.0 - shipPlan.freeThreat) * rule.maxThreatFactor) or 0.0

		for _, slot in ipairs(slots) do

			if EquipSet.CompatibleWithSlot(equip, slot) and (shipPlan.freeThreat - reserveThreat) >= threat then

				local inst = equip:Instance()

				if inst.SpecializeForShip then
					inst:SpecializeForShip(shipConfig)
				end

				if slot.count then
					inst:SetCount(slot.count)
				end

				if rule.apply then
					rule.apply(inst)
				end

				if shipPlan.freeVolume >= inst.volume then
					shipPlan:AddEquipToPlan(equip, slot, threat)
				end

				numInstalled = numInstalled + 1

				if rule.limit and numInstalled >= rule.limit then
					break
				end

			end

		end

		return
	end

	-- Limit equipment according to what will actually fit the ship
	-- NOTE: this does not guarantee all slots will be able to be filled in a balanced manner
	local maxVolume = rule.balance and shipPlan.freeVolume / #slots or shipPlan.freeVolume

	-- Limit maximum threat consumption of this equipment rule
	local allowedThreat = (rule.maxThreatFactor or 1.0) * shipPlan.freeThreat
	local reserveThreat = shipPlan.freeThreat - allowedThreat
	local maxThreat = rule.balance and allowedThreat / #slots or allowedThreat

	-- Build a list of all equipment items that could potentially be installed
	local filteredEquip = utils.to_array(Equipment.new, function(equip)
		return (equip.slot or false)
			and matchRuleSlot(equip.slot, rule.filter or rule.slot)
			and equip.volume <= maxVolume
	end)

	-- print("Available equipment: " .. #filteredEquip)

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
		---@type EquipType[]
		local compatible = utils.map_array(filteredEquip, function(equip)
			local threat = threatCache[equip]
			local withinThreat = threat == 0
				or threat <= (shipPlan.freeThreat - reserveThreat)
				and threat <= maxThreat

			local compat = EquipSet.CompatibleWithSlot(equip, slot) and withinThreat

			if not compat then
				return nil
			end

			local inst = equip:Instance()

			if inst.SpecializeForShip then
				inst:SpecializeForShip(shipConfig)
			end

			if slot.count then
				inst:SetCount(slot.count)
			end

			if rule.apply then
				rule.apply(inst)
			end

			return shipPlan.freeVolume >= inst.volume and inst or nil
		end)

		-- print("Slot " .. slot.id .. " - compatible: " .. #compatible)

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

			if rule.limit and numInstalled >= rule.limit then
				break
			end

		end

	end

end

-- =============================================================================

-- Function: GetHullThreat
--
-- Return the threat factor table computed for a given hull configuration,
-- looked up by the passed identifier.
function ShipBuilder.GetHullThreat(shipId)
	return hullThreatCache[shipId] or { total = 0.0 }
end

-- Function: SelectHull
--
-- Return a ship hull configuration appropriate for the passed template and
-- threat factor.
--
-- If the passed template specifies a hull configuration identifier in the
-- shipId field, that configuration is returned directly. Otherwise, the list
-- of available hull configurations is filtered and a random valid hull is
-- returned.
--
-- Parameters:
--
--  template - MissionUtils.ShipTemplate, used to filter the selection of hulls
--             by role, hyperdrive class, etc.
--
--  threat   - number, intended encounter threat factor used to filter valid
--             hulls by their threat factor. This value is used to compute both
--             an upper and lower bound on allowable threat value for the hull.
--
-- Returns:
--
--  hull - HullConfig?, the hull configuration selected for this template, or
--         nil if no hulls passed the validity checks specified by the threat
--         factor and ship template.
--
---@param template MissionUtils.ShipTemplate
---@param threat number
---@return HullConfig?
function ShipBuilder.SelectHull(template, threat)

	local hullList = {}

	if template.shipId then

		table.insert(hullList, template.shipId)

	else

		for id, shipDef in pairs(ShipDef) do

			local acceptable = shipDef.tag == "SHIP"
				and (not template.role or shipDef.roles[template.role])
				and (not template.hyperclass or shipDef.hyperdriveClass >= template.hyperclass)

			if acceptable then

				local hullThreat = ShipBuilder.GetHullThreat(id).total

				-- Use threat metric as a way to balance the random selection of ship hulls
				local withinRange = hullThreat <= ShipBuilder.kMaxHullThreatFactor * threat
					and hullThreat >= ShipBuilder.kMinHullThreatFactor * threat

				-- print(id, hullThreat, threat, withinRange)

				if withinRange then
					table.insert(hullList, id)
				end

			end

		end

	end

	if #hullList == 0 then
		return nil
	end

	local hullIdx = Engine.rand:Integer(1, #hullList)
	local shipId = hullList[hullIdx]

	-- print("  threat {} => {} ({} / {})" % { threat, shipId, hullIdx, #hullList })

	return HullConfig.GetHullConfig(shipId)

end

-- Function: MakePlan
--
-- Evaluates all equipment rules specified in the ship template to produce a
-- concrete plan for equipping the given hull configuration.
--
-- Rules are evaluated in order of appearance, and can be disabled by the
-- minThreat and randomChance rule parameters in each individual equipment rule.
--
-- See data/modules/MissionUtils/OutfitRules.lua for information on the fields
-- applicable to an equipment rule.
--
-- Parameters:
--
--  template   - MissionUtils.ShipTemplate, the template containing equipment
--               rules to evaluate.
--
--  shipConfig - HullConfig, the hull configuration to make a plan for.
--
--  threat     - number, controls the intended difficulty of the encounter. The
--               threat value is used to limit which equipment can be installed
--               and whether an equipment rule can be evaluated at all via the
--               minThreat property.
--
-- Returns:
--
--  plan - table, an opaque structure containing information about the ship
--         to spawn and the equipment to install as a result of evaluating the
--         input template.
--
---@param template MissionUtils.ShipTemplate
---@param shipConfig HullConfig
---@param threat number
---@return MissionUtils.ShipBuilder.ShipPlan
function ShipBuilder.MakePlan(template, shipConfig, threat)

	local hullThreat = ShipBuilder.GetHullThreat(shipConfig.id).total

	local randomMod = template.randomModifier or 1.0

	local shipPlan = ShipPlan:clone {
		threat = hullThreat,
		freeThreat = threat - hullThreat,
		maxThreat = threat
	}

	shipPlan:SetConfig(shipConfig)

	if shipPlan.freeThreat <= 0 then
		logWarning("ShipBuilder: {} has a hull threat of {}, greater than the provided threat {}. Ship will not be properly outfitted." % {
			shipConfig.id, hullThreat, threat})
	end

	-- Setup required equipment items for the ship
	-- TODO: support op="replace" equipment rules to override required slots?
	for _, slot in pairs(shipPlan.slots) do

		if slot.required then
			local defaultEquip = Equipment.Get(slot.default)

			if defaultEquip then

				local inst = defaultEquip:Instance()

				if inst.SpecializeForShip then
					inst:SpecializeForShip(shipPlan.config)
				end

				if slot.count then
					inst:SetCount(slot.count)
				end

				shipPlan:AddEquipToPlan(inst, slot)
			end
		end

	end

	for _, rule in ipairs(template.rules) do

		local canApplyRule = true

		if rule.minThreat then canApplyRule = canApplyRule and threat >= rule.minThreat end
		if rule.randomChance then canApplyRule = canApplyRule and Engine.rand:Number() < rule.randomChance * randomMod end

		if canApplyRule then

			if rule.slot then
				ShipBuilder.ApplyEquipmentRule(shipPlan, rule, Engine.rand, hullThreat)
			else
				local inst = assert(Equipment.Get(rule.equip)):Instance()

				if inst.SpecializeForShip then
					inst:SpecializeForShip(shipPlan.config)
				end

				if rule.apply then
					rule.apply(inst)
				end

				local equipThreat = ShipBuilder.ComputeEquipThreatFactor(inst, hullThreat)

				if shipPlan.freeVolume >= inst.volume and shipPlan.freeThreat >= equipThreat then
					shipPlan:AddEquipToPlan(inst)
				end
			end

		end

	end

	shipPlan.label = template.label or Ship.MakeRandomLabel()

	return shipPlan

end

-- Function: ApplyPlan
--
-- Apply the previously created plan to a concrete ship object of the
-- previously-specified hull configuration, installing all equipment instances
-- created by applying the equipment rules contained in the ship template.
--
-- This function ensures that equipment is properly installed in the order it
-- was selected and items are properly distributed to sub-slots provided by
-- installed equipment items.
--
-- Parameters:
--
--  ship     - Ship, the ship to install equipment into.
--
--  shipPlan - table, an opaque ship plan returned from a previous call to
--             ShipBuilder.MakePlan.
--
---@param ship Ship
---@param shipPlan MissionUtils.ShipBuilder.ShipPlan
function ShipBuilder.ApplyPlan(ship, shipPlan)

	assert(ship.shipId == shipPlan.shipId, "Applying a ship plan to an incompatible ship!")

	local equipSet = ship:GetComponent('EquipSet')

	-- Apply slot-based equipment first
	for _, slotId in ipairs(shipPlan.install) do
		local slot = assert(equipSet:GetSlotHandle(slotId))
		local equip = assert(shipPlan.filled[slotId])
		assert(not equip:isProto())

		equipSet:Install(equip, slot)
	end

	for _, equip in ipairs(shipPlan.equip) do
		assert(not equip:isProto())
		equipSet:Install(equip)
	end

	-- TODO: ammunition / other items inside of instanced equipment

	ship:SetLabel(shipPlan.label)

end

-- =============================================================================

-- Function: MakeShipNear
--
-- Spawns a ship near the specified body according to the given ship template
-- and threat value.
--
-- See: Space.SpawnShipNear
---@param body Body
---@param template MissionUtils.ShipTemplate
---@param threat number?
---@param nearDist number?
---@param farDist number?
---@return Ship
function ShipBuilder.MakeShipNear(body, template, threat, nearDist, farDist)
	if not threat then
		threat = Engine.rand:Number(ShipBuilder.kDefaultRandomThreatMin, ShipBuilder.kDefaultRandomThreatMax)
	end

	local hullConfig = ShipBuilder.SelectHull(template, threat)
	assert(hullConfig)

	local plan = ShipBuilder.MakePlan(template, hullConfig, threat)
	assert(plan)

	local ship = Space.SpawnShipNear(plan.shipId, body, nearDist or 50, farDist or 100)
	assert(ship)

	ShipBuilder.ApplyPlan(ship, plan)

	return ship
end

-- Function: MakeShipOrbit
--
-- Spawns a ship in orbit around a specified body according to the given ship
-- template and threat value.
--
-- See: Spawn.SpawnShipOrbit
---@param body Body
---@param template MissionUtils.ShipTemplate
---@param threat number
---@param nearDist number
---@param farDist number
---@return Ship
function ShipBuilder.MakeShipOrbit(body, template, threat, nearDist, farDist)
	if not threat then
		threat = Engine.rand:Number(ShipBuilder.kDefaultRandomThreatMin, ShipBuilder.kDefaultRandomThreatMax)
	end

	local hullConfig = ShipBuilder.SelectHull(template, threat)
	assert(hullConfig)

	local plan = ShipBuilder.MakePlan(template, hullConfig, threat)
	assert(plan)

	local ship = Space.SpawnShipOrbit(plan.shipId, body, nearDist, farDist)
	assert(ship)

	ShipBuilder.ApplyPlan(ship, plan)

	return ship
end

-- Function: MakeShipLanded
--
-- Spawns a ship landed on a specified body according to the given ship
-- template and threat value.
--
-- See: Spawn.SpawnShipLanded
---@param body Body
---@param template MissionUtils.ShipTemplate
---@param threat number
---@param lat number
---@param lon number
---@return Ship
function ShipBuilder.MakeShipLanded(body, template, threat, lat, lon)
	if not threat then
		threat = Engine.rand:Number(ShipBuilder.kDefaultRandomThreatMin, ShipBuilder.kDefaultRandomThreatMax)
	end

	local hullConfig = ShipBuilder.SelectHull(template, threat)
	assert(hullConfig)

	local plan = ShipBuilder.MakePlan(template, hullConfig, threat)
	assert(plan)

	local ship = Space.SpawnShipLanded(plan.shipId, body, lat, lon)
	assert(ship)

	ShipBuilder.ApplyPlan(ship, plan)

	return ship
end

-- Function: MakeShipDocked
--
-- Spawns a ship docked atan a specified station according to the given ship
-- template and threat value.
--
-- See: Spawn.SpawnShipDocked
---@param body SpaceStation
---@param template MissionUtils.ShipTemplate
---@param threat number?
---@return Ship?
function ShipBuilder.MakeShipDocked(body, template, threat)
	if not threat then
		threat = Engine.rand:Number(ShipBuilder.kDefaultRandomThreatMin, ShipBuilder.kDefaultRandomThreatMax)
	end

	local hullConfig = ShipBuilder.SelectHull(template, threat)
	assert(hullConfig)

	local plan = ShipBuilder.MakePlan(template, hullConfig, threat)
	assert(plan)

	local ship = Space.SpawnShipDocked(plan.shipId, body)

	-- All docks may be full...
	if not ship then
		return nil
	end

	ShipBuilder.ApplyPlan(ship, plan)

	return ship
end

-- Function: MakeShipAroundStar
--
-- Spawns a ship in orbit around the system center according to the given ship
-- template and threat value.
--
-- See: Spawn.SpawnShip
---@param template MissionUtils.ShipTemplate
---@param threat number
---@param minDistAu number
---@param maxDistAu number
---@return Ship
function ShipBuilder.MakeShipAroundStar(template, threat, minDistAu, maxDistAu)
	if not threat then
		threat = Engine.rand:Number(ShipBuilder.kDefaultRandomThreatMin, ShipBuilder.kDefaultRandomThreatMax)
	end

	local hullConfig = ShipBuilder.SelectHull(template, threat)
	assert(hullConfig)

	local plan = ShipBuilder.MakePlan(template, hullConfig, threat)
	assert(plan)

	local ship = Space.SpawnShip(plan.shipId, minDistAu or 1, maxDistAu or 10)
	assert(ship)

	ShipBuilder.ApplyPlan(ship, plan)

	return ship
end

-- =============================================================================

-- Generate a cache of combined hull threat factor for each ship in the game
function ShipBuilder.BuildHullThreatCache()
	for id, shipDef in pairs(ShipDef) do
		local threat = ShipBuilder.ComputeHullThreatFactor(shipDef)

		hullThreatCache[id] = threat
	end
end

Event.Register("onGameStart", function()
	ShipBuilder.BuildHullThreatCache()
end)

return ShipBuilder
