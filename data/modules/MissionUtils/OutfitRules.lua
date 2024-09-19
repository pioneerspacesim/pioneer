-- Copyright © 2008-2023 Pioneer Developers. See AUTHORS.txt for details
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
---@field balance boolean? Attempt to balance volume / threat across all slots this rule matches (works best with .pick = nil)
---@field maxThreatFactor number? Maximum proportion of remaining threat that can be consumed by this rule
---@field minThreat number? Minimum threat value for the entire ship that has to be met to consider this rule

OutfitRules.DefaultHyperdrive = {
	slot = "hyperdrive"
}

OutfitRules.DefaultAtmoShield = {
	slot = "hull",
	filter = "hull.atmo_shield",
	limit = 1
}

OutfitRules.DefaultShieldGen = {
	slot = "shield",
	limit = 1
}

OutfitRules.DefaultAutopilot = {
	slot = "computer",
	equip = "misc.autopilot",
	limit = 1
}

OutfitRules.DefaultLaserCooling = {
	slot = nil,
	equip = "misc.laser_cooling_booster",
	limit = 1
}

return OutfitRules
