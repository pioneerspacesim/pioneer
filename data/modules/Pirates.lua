-- Copyright © 2008-2014 Pioneer Developers. See AUTHORS.txt for details
-- Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

local Engine = import("Engine")
local Game = import("Game")
local Space = import("Space")
local Event = import("Event")
local Equipment = import("Equipment")
local ShipDef = import("ShipDef")
local Ship = import("Ship")
local utils = import("utils")

local onEnterSystem = function (player)
	if not player:IsPlayer() then return end

	local shipdefs = utils.build_array(utils.filter(function (k,def) return def.tag == 'SHIP'
		and def.hyperdriveClass > 0 and def.hullMass <= 150 end, pairs(ShipDef)))
	if #shipdefs == 0 then return end

	local lawlessness = Game.system.lawlessness

	-- XXX number should be some combination of population, lawlessness,
	-- proximity to shipping lanes, etc
	local max_pirates = 6
	while max_pirates > 0 and Engine.rand:Number(1) < lawlessness do
		max_pirates = max_pirates-1

		local shipdef = shipdefs[Engine.rand:Integer(1,#shipdefs)]
		local drive_class = shipdef.hyperdriveClass
		local default_drive = drive_class > 0 and Equipment.hyperspace['drive_class'..tostring(drive_class)]

		-- select a laser. this is naive - it simply chooses at random from
		-- the set of lasers that will fit, but never more than one above the
		-- player's current weapon.
		-- XXX this should use external factors (eg lawlessness) and not be
		-- dependent on the player in any way
		local max_laser_size = shipdef.capacity - (drive_class > 0 and default_drive.capabilities.mass or 0)
		local laserdefs = utils.build_array(utils.filter(
			function (k,l) return l:IsValidSlot('laser_front') and l.capabilities.mass <= max_laser_size and string.sub(k.id,0,11) == 'pulsecannon' end,
			pairs(Equipment.laser)
		))
		local laserdef = laserdefs[Engine.rand:Integer(1,#laserdefs)]

		local ship = Space.SpawnShip(shipdef.id, 8, 12)
		ship:SetLabel(Ship.MakeRandomLabel())
		if drive_class > 0 then
			ship:AddEquip(default_drive)
		end
		ship:AddEquip(laserdef)

		local playerCargoCapacity = ShipDef[player.shipId].capacity
		local probabilityPirateIsInterested = playerCargoCapacity/100.0
		if Engine.rand:Number(1) < probabilityPirateIsInterested then
			ship:AIKill(Game.player)
		end
	end
end

Event.Register("onEnterSystem", onEnterSystem)
