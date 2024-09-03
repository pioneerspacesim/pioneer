-- Copyright © 2008-2024 Pioneer Developers. See AUTHORS.txt for details
-- Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

local Event       = require 'Event'
local Commodities = require 'Commodities'
local Game        = require 'Game'
local Lang        = require 'Lang'

local lc = Lang.GetResource("core")

-- Utility functions to handle fuel and cargo scooping.
-- These callbacks are triggered from C++ or Lua and are responsible for
-- providing UI feedback about the scooping process.

Event.Register("onShipScoopFuel", function(ship, body, amount)
	---@type CargoManager
	local cargoMgr = ship:GetComponent('CargoManager')

	cargoMgr:AddCommodity(Commodities.hydrogen, amount)

	if ship == Game.player then
		Game.AddCommsLogLine(lc.FUEL_SCOOP_ACTIVE_N_TONNES_H_COLLECTED:gsub('%%quantity',
			cargoMgr:CountCommodity(Commodities.hydrogen)))
	end
end)

Event.Register("onShipScoopCargo", function (ship, success, cargoType)
	if ship ~= Game.player then return end

	if success then
		Game.AddCommsLogLine(lc.CARGO_SCOOP_ACTIVE_1_TONNE_X_COLLECTED:gsub('%%item',
			cargoType:GetName()))
	else
		Game.AddCommsLogLine(lc.CARGO_SCOOP_ATTEMPTED)
	end
end)
