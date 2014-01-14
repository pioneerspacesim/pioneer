-- Copyright © 2008-2014 Pioneer Developers. See AUTHORS.txt for details
-- Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

local Engine = import("Engine")
local Game = import("Game")
local Space = import("Space")
local Event = import("Event")
local EquipDef = import("EquipDef")
local ShipDef = import("ShipDef")
local Ship = import("Ship")
local utils = import("utils")

local onEnterSystem = function (player)
	if not player:IsPlayer() then return end

	local shipdefs = utils.build_array(utils.filter(function (k,def) return def.tag == 'SHIP' and def.hullMass <= 150 end, pairs(ShipDef)))
	if #shipdefs == 0 then return end

	local lawlessness = Game.system.lawlessness

	-- XXX number should be some combination of population, lawlessness,
	-- proximity to shipping lanes, etc
	local max_pirates = 6
	while max_pirates > 0 and Engine.rand:Number(1) < lawlessness do
		max_pirates = max_pirates-1

		local shipdef = shipdefs[Engine.rand:Integer(1,#shipdefs)]
		local default_drive = shipdef.defaultHyperdrive

		-- select a laser. this is naive - it simply chooses at random from
		-- the set of lasers that will fit, but never more than one above the
		-- player's current weapon.
		-- XXX this should use external factors (eg lawlessness) and not be
		-- dependent on the player in any way
		local max_laser_size = shipdef.capacity - EquipDef[default_drive].mass
		local laserdefs = utils.build_array(utils.filter(function (k, def) return def.slot == 'LASER' and def.mass <= max_laser_size and string.sub(def.id,0,11) == 'PULSECANNON' end, pairs(EquipDef)))
		local laserdef = laserdefs[Engine.rand:Integer(1,#laserdefs)]

		local ship = Space.SpawnShip(shipdef.id, 8, 12)
		ship:SetLabel(Ship.MakeRandomLabel())
		ship:AddEquip(default_drive)
		ship:AddEquip(laserdef.id)

		local playerCargoCapacity = ShipDef[player.shipId].capacity
		local probabilityPirateIsInterested = playerCargoCapacity/100.0
		if Engine.rand:Number(1) < probabilityPirateIsInterested then
			ship:AIKill(Game.player)
		end
	end
end

Event.Register("onEnterSystem", onEnterSystem)
