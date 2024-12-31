-- Copyright © 2008-2025 Pioneer Developers. See AUTHORS.txt for details
-- Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

local Engine = require 'Engine'
local Game = require 'Game'
local Event = require 'Event'
local ShipDef = require 'ShipDef'
local utils = require 'utils'

local MissionUtils = require 'modules.MissionUtils'
local ShipBuilder  = require 'modules.MissionUtils.ShipBuilder'

local onEnterSystem = function (player)
	if not player:IsPlayer() then return end

	local shipdefs = utils.build_array(utils.filter(function (k,def) return def.tag == 'SHIP'
		and def.hyperdriveClass > 0 and def.roles.pirate end, pairs(ShipDef)))
	if #shipdefs == 0 then return end

	local lawlessness = Game.system.lawlessness

	-- XXX number should be some combination of population, lawlessness,
	-- proximity to shipping lanes, etc
	local max_pirates = 6
	while max_pirates > 0 do
		max_pirates = max_pirates-1

		if Engine.rand:Number(1) < lawlessness then
			-- Simple threat calculation based on lawlessness factor and independent of the player's current state.
			-- This will likely not produce an assailant larger than a Deneb.
			local threat = 10.0 + Engine.rand:Number(100.0 * lawlessness)

			local ship = ShipBuilder.MakeShipAroundStar(MissionUtils.ShipTemplates.GenericPirate, threat, 8, 12)

			-- pirates know how big cargo hold the ship model has
			local playerCargoCapacity = ShipDef[player.shipId].equipCapacity

			-- Pirate attack probability proportional to how fully loaded cargo hold is.
			local discount = 2 		-- discount on 2t for small ships.
			local probabilityPirateIsInterested = math.floor(player.usedCargo - discount) / math.max(1,  playerCargoCapacity - discount)

			if Engine.rand:Number(1) <= probabilityPirateIsInterested then
				ship:AIKill(Game.player)
			end
		end
	end
end

Event.Register("onEnterSystem", onEnterSystem)
