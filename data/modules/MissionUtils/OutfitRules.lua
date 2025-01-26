-- Copyright © 2008-2025 Pioneer Developers. See AUTHORS.txt for details
-- Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

---@class MissionUtils.OutfitRules
local OutfitRules = {}

---@class MissionUtils.OutfitRule
---@field slot string? Slot type filter string
---@field equip string? Explicit equipment item to install in filtered slots
---@field filter string? Filter for random equipment types
---@field limit integer? Maximum number of equipment items to install
---@field pick nil | "random" Pick the biggest/best item for the slot, or a random compatible item
---@field maxSize integer? Limit the maximum size of items equipped by this rule
---@field minSize integer? Limit the minimum size of items equipped by this rule
---@field minThreat number? Minimum threat value for the entire ship that has to be met to consider this rule
---@field maxThreatFactor number? Maximum proportion of remaining threat that can be consumed by this rule
---@field randomChance number? Random chance to apply this rule, in [0..1]
---@field balance boolean? Attempt to balance volume / threat across all slots this rule matches (works best with .pick = nil)
---@field apply fun(equip: EquipType)? Make custom changes to the selected equipment before it is installed in the ship

OutfitRules.DifficultWeapon = {
	slot = "weapon",
	minSize = 2,
	minThreat = 50.0,
	maxThreatFactor = 0.7,
	balance = true,
}

OutfitRules.ModerateWeapon = {
	slot = "weapon",
	limit = 2,
	maxSize = 3,
	minThreat = 30.0,
	maxThreatFactor = 0.6,
	balance = true,
}

OutfitRules.EasyWeapon = {
	slot = "weapon",
	maxSize = 2,
	maxThreatFactor = 0.5,
	balance = true,
}

OutfitRules.PulsecannonModerateWeapon = {
	slot = "weapon",
	filter = "weapon.energy.pulsecannon",
	limit = 2,
	maxSize = 3,
	minThreat = 30.0,
	maxThreatFactor = 0.6,
	balance = true
}

OutfitRules.PulsecannonEasyWeapon = {
	slot = "weapon",
	filter = "weapon.energy.pulsecannon",
	limit = 2,
	maxSize = 3,
	maxThreatFactor = 0.5,
	balance = true
}

OutfitRules.DifficultShieldGen = {
	slot = "shield",
	minSize = 2,
	minThreat = 50.0,
	balance = true
}

OutfitRules.ModerateShieldGen = {
	slot = "shield",
	maxSize = 3,
	limit = 2,
	minThreat = 20.0,
	maxThreatFactor = 0.8,
	balance = true
}

OutfitRules.EasyShieldGen = {
	slot = "shield",
	maxSize = 2,
	limit = 1,
	minThreat = 20.0,
	maxThreatFactor = 0.6
}

-- Default rules always equip the item if there's enough space

OutfitRules.DefaultPassengerCabins = {
	slot = "cabin",
	filter = "cabin.passenger"
}

OutfitRules.AnyHyperdrive = {
	slot = "hyperdrive",
	apply = function(equip) ---@param equip Equipment.HyperdriveType
		equip:SetFuel(nil, equip:GetMaxFuel())
	end
}

OutfitRules.DefaultHyperdrive = {
	slot = "hyperdrive",
	filter = "hyperdrive.civilian",
	apply = function(equip) ---@param equip Equipment.HyperdriveType
		equip:SetFuel(nil, equip:GetMaxFuel())
	end
}

OutfitRules.DefaultAtmoShield = {
	slot = "hull",
	filter = "hull.atmo_shield",
	limit = 1
}

OutfitRules.DefaultShieldGen = {
	slot = "shield"
}

OutfitRules.DefaultAutopilot = {
	slot = "computer",
	equip = "misc.autopilot",
	limit = 1
}

OutfitRules.DefaultShieldBooster = {
	equip = "misc.shield_energy_booster",
	limit = 1
}

OutfitRules.DefaultLaserCooling = {
	slot = nil,
	equip = "misc.laser_cooling_booster",
	limit = 1
}

OutfitRules.DefaultRadar = {
	slot = "sensor",
	filter = "sensor.radar",
	limit = 1
}

return OutfitRules
