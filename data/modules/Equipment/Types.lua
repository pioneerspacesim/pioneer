-- Copyright © 2008-2025 Pioneer Developers. See AUTHORS.txt for details
-- Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

local EquipType = require 'EquipType'.EquipType
local ShipDef   = require 'ShipDef'

local utils = require 'utils'

--==============================================================================

---@class Equipment.CargoScoopType : EquipType
local CargoScoopType = EquipType:NewType("Equipment.CargoScoopType")

CargoScoopType.volumePerSize = 1.5
CargoScoopType.massPerSize = 1.5
CargoScoopType.pricePerCargo = 10
CargoScoopType.pricePerSize = 0.4

---@param config HullConfig
function CargoScoopType:SpecializeForShip(config)
	local def = ShipDef[config.id]
	if not def then return end

	local hullSlot = utils.best_score(config.slots, function(k, v)
		return v.type == "hull" and 1 or nil
	end)

	if not hullSlot then return end

	-- Effective ship size, clamped to S1
	local size = math.max(hullSlot.size, 1)

	local cargoPriceMod = def.cargo * self.pricePerCargo

	local sizeMassMod = (size - 1) * self.massPerSize * self.mass
	local sizePriceMod = (size - 1) * self.pricePerSize * self.price
	local sizeVolumeMod = (size - 1) * self.volumePerSize * self.volume

	self.volume = utils.round(self.volume + sizeVolumeMod, 0.5)
	self.mass = utils.round(self.mass + sizeMassMod, 0.1)
	self.price = self.price + utils.round(sizePriceMod + cargoPriceMod, 10.0)
end

--==============================================================================

return {
	CargoScoopType = CargoScoopType
}
