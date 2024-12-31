-- Copyright © 2008-2025 Pioneer Developers. See AUTHORS.txt for details
-- Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

local Commodities = require 'Commodities'
local Game        = require 'Game'
local Engine      = require 'Engine'
local Event       = require 'Event'
local Timer       = require 'Timer'

local lc = require 'Lang'.GetResource("core")

local function LifeSupportCallback(self)
	if not self:exists() then return true end

	if self:GetDockedWith() then
		return false
	end

	---@type CargoManager
	local cargoMgr = self:GetComponent('CargoManager')

	local commodityName = cargoMgr:DoLifeSupportChecks(self.cargo_life_support_cap or 0)
	if commodityName then
		cargoMgr:RemoveCommodity(Commodities[commodityName], 1)

		-- randomly convert the commodity into a suitable alternative
		-- TODO: have a more generic check instead of hard-coding the only two commodities which current require life-support.
		local dice = 0
		if commodityName == "live_animals" then
			dice = Engine.rand:Integer(0,3)
		elseif commodityName == "slaves" then
			dice = Engine.rand:Integer(0,2)
		end
		if dice == 3 then
			commodityName = "animal_meat"
		elseif dice == 2 then
			commodityName = "fertilizer"
		else
			commodityName = "rubbish"
		end
		cargoMgr:AddCommodity(Commodities[commodityName], 1)

		if self == Game.player then
			Game.AddCommsLogLine(lc.CARGO_BAY_LIFE_SUPPORT_LOST)
		end
	end

	-- if self == Game.player then
	-- 	Engine.RequestProfileFrame()
	-- end

	return false
end

-- TODO: this can be massively improved as far as performance goes; it should
-- only start the timer when a ship is undocked with perishable cargo on board,
-- and cancel the timer when the ship redocks.
--
-- Currently, performance in the average case is quite respectable: a typical
-- load of ~200us on a decently old machine with 400+ ships.
-- Updating LuaTimer to have less overhead while checking for time expiration
-- would also help optimize this.
Event.Register('onShipCreated', function (ship)
	-- Many ships can be created during the same frame (by e.g. Tradeships)
	-- so distribute timer callback load over the full 5s to avoid massive
	-- single-frame spikes where every ship is processed at once
	Timer:CallAt(Game.time + 5.0 * Engine.rand:Number(), function()
		Timer:CallEvery(5.0, function() return LifeSupportCallback(ship) end)
	end)
end)
