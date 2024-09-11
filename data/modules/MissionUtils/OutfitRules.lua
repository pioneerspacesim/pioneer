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

---@class MissionUtils.OutfitRules
local OutfitRules = {}

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
