-- Copyright © 2008-2024 Pioneer Developers. See AUTHORS.txt for details
-- Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

local Engine = require 'Engine'
local Game = require 'Game'
local Space = require 'Space'
local Event = require 'Event'
local Equipment = require 'Equipment'
local ShipDef = require 'ShipDef'
local Ship = require 'Ship'
local utils = require 'utils'

local onEnterSystem = function (player)
	if not player:IsPlayer() then return end

	local shipdefs = utils.build_array(utils.filter(function (k,def) return def.tag == 'SHIP'
		and def.hyperdriveClass > 0 and def.roles.pirate end, pairs(ShipDef)))
	if #shipdefs == 0 then return end

	local lawlessness = Game.system.lawlessness

	-- XXX number should be some combination of population, lawlessness,
	-- proximity to shipping lanes, etc
	local max_pirates = 6
	while max_pirates > 0 and Engine.rand:Number(1) < lawlessness do
		max_pirates = max_pirates-1

		local shipdef = shipdefs[Engine.rand:Integer(1,#shipdefs)]
		local default_drive = Equipment.hyperspace['hyperdrive_'..tostring(shipdef.hyperdriveClass)]
		assert(default_drive)  -- never be nil.

		-- select a laser. this is naive - it simply chooses at random from
		-- the set of lasers that will fit, but never more than one above the
		-- player's current weapon.
		-- XXX this should use external factors (eg lawlessness) and not be
		-- dependent on the player in any way
		local max_laser_size = shipdef.capacity - default_drive.capabilities.mass
		local laserdefs = utils.build_array(utils.filter(
			function (k,l) return l:IsValidSlot('laser_front') and l.capabilities.mass <= max_laser_size and l.l10n_key:find("PULSECANNON") end,
			pairs(Equipment.laser)
		))
		local laserdef = laserdefs[Engine.rand:Integer(1,#laserdefs)]

		local ship = Space.SpawnShip(shipdef.id, 8, 12)
		ship:SetLabel(Ship.MakeRandomLabel())
		ship:AddEquip(default_drive)
		ship:AddEquip(laserdef)

		-- pirates know how big cargo hold the ship model has
		local playerCargoCapacity = ShipDef[player.shipId].capacity

		-- Pirate attack probability proportional to how fully loaded cargo hold is.
		local discount = 2 		-- discount on 2t for small ships.
		local probabilityPirateIsInterested = math.floor(player.usedCargo - discount) / math.max(1,  playerCargoCapacity - discount)

		if Engine.rand:Number(1) <= probabilityPirateIsInterested then
			ship:AIKill(Game.player)
		end
	end
end

Event.Register("onEnterSystem", onEnterSystem)
