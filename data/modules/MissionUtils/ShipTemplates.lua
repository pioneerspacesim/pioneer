-- Copyright © 2008-2025 Pioneer Developers. See AUTHORS.txt for details
-- Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

local ShipBuilder = require 'modules.MissionUtils.ShipBuilder'

local utils = require 'utils'

local OutfitRules = ShipBuilder.OutfitRules

---@class MissionUtils.ShipTemplates
local ShipTemplates = {}

ShipTemplates.StrongPirate = ShipBuilder.Template:clone {
	role = "pirate",
	hyperclass = 1,
	rules = {
		-- If the pirate is threatening enough, it can have any weapon,
		-- otherwise it will get a simple spread of pulsecannons
		OutfitRules.DifficultWeapon,
		OutfitRules.PulsecannonModerateWeapon,
		OutfitRules.PulsecannonEasyWeapon,
		-- Equip shield generators in descending order of difficulty
		OutfitRules.DifficultShieldGen,
		OutfitRules.ModerateShieldGen,
		OutfitRules.EasyShieldGen,
		-- Potentially throw laser cooling and a shield booster in the mix
		utils.mixin(OutfitRules.DefaultLaserCooling, { minThreat = 40.0 }),
		utils.mixin(OutfitRules.DefaultShieldBooster, { minThreat = 30.0 }),
		OutfitRules.DefaultHyperdrive,
		OutfitRules.DefaultAtmoShield,
		OutfitRules.DefaultAutopilot,
		OutfitRules.DefaultRadar,
	}
}

ShipTemplates.GenericPirate = ShipBuilder.Template:clone {
	role = "pirate",
	hyperclass = 1,
	rules = {
		OutfitRules.PulsecannonModerateWeapon,
		OutfitRules.EasyWeapon,
		OutfitRules.ModerateShieldGen,
		OutfitRules.EasyShieldGen,
		OutfitRules.DefaultHyperdrive,
		OutfitRules.DefaultAtmoShield,
		utils.mixin(OutfitRules.DefaultLaserCooling, { minThreat = 40.0 }),
		utils.mixin(OutfitRules.DefaultShieldBooster, { minThreat = 30.0 }),
		OutfitRules.DefaultAutopilot,
		OutfitRules.DefaultRadar,
	}
}

ShipTemplates.WeakPirate = ShipBuilder.Template:clone {
	role = "pirate",
	hyperclass = 1,
	rules = {
		OutfitRules.PulsecannonModerateWeapon,
		OutfitRules.PulsecannonEasyWeapon,
		-- Just a basic shield generator on the tougher ones
		OutfitRules.EasyShieldGen,
		-- No laser cooling or shield booster to be found here
		OutfitRules.DefaultHyperdrive,
		OutfitRules.DefaultAtmoShield,
		OutfitRules.DefaultAutopilot,
		OutfitRules.DefaultRadar,
	}
}

ShipTemplates.GenericPolice = ShipBuilder.Template:clone {
	role = "police",
	hyperclass = 1,
	rules = {
		OutfitRules.ModerateWeapon,
		OutfitRules.EasyWeapon,
		OutfitRules.ModerateShieldGen,
		OutfitRules.EasyShieldGen,
		utils.mixin(OutfitRules.DefaultShieldBooster, { minThreat = 30.0 }),
		utils.mixin(OutfitRules.DefaultLaserCooling, { minThreat = 20.0 }),
		OutfitRules.DefaultHyperdrive,
		OutfitRules.DefaultAtmoShield,
		OutfitRules.DefaultAutopilot,
		OutfitRules.DefaultRadar,
	}
}

ShipTemplates.StationPolice = ShipBuilder.Template:clone {
	role = "police",
	rules = {
		{
			slot = "weapon",
			equip = "laser.pulsecannon_dual_1mw",
			limit = 1
		},
		OutfitRules.ModerateWeapon,
		OutfitRules.EasyWeapon,
		OutfitRules.ModerateShieldGen,
		-- Always has laser cooling but no need for hyperdrive
		OutfitRules.DefaultLaserCooling,
		OutfitRules.DefaultAtmoShield,
		OutfitRules.DefaultAutopilot,
		OutfitRules.DefaultRadar,
	}
}

ShipTemplates.PolicePatrol = ShipBuilder.Template:clone {
	role = "police",
	rules = {
		{
			slot = "weapon",
			equip = "laser.pulsecannon_1mw",
			limit = 1
		},
		OutfitRules.ModerateWeapon,
		OutfitRules.EasyWeapon,
		OutfitRules.ModerateShieldGen,
		OutfitRules.EasyShieldGen,
		OutfitRules.DefaultAtmoShield,
		OutfitRules.DefaultAutopilot,
		OutfitRules.DefaultRadar,
	}
}

ShipTemplates.GenericMercenary = ShipBuilder.Template:clone {
	role = "mercenary",
	hyperclass = 1,
	rules = {
		OutfitRules.DifficultWeapon,
		OutfitRules.ModerateWeapon,
		OutfitRules.EasyWeapon,
		-- Set shield gens according to mission difficulty
		OutfitRules.DifficultShieldGen,
		OutfitRules.ModerateShieldGen,
		OutfitRules.EasyShieldGen,
		-- Enable add-on equipment based on mission difficulty
		utils.mixin(OutfitRules.DefaultLaserCooling, { minThreat = 40.0 }),
		utils.mixin(OutfitRules.DefaultShieldBooster, { minThreat = 30.0 }),
		-- Default equipment in remaining space
		OutfitRules.AnyHyperdrive,
		OutfitRules.DefaultAtmoShield,
		OutfitRules.DefaultRadar,
	}
}

return ShipTemplates
