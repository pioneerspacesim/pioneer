local onEnterSystem = function (player)
	if not player:IsPlayer() then return end

	local shiptypes = ShipType.GetShipTypes('SHIP', function (t)
		local mass = t.hullMass
		return mass >= 50 and mass <= 150
	end)
	if #shiptypes == 0 then return end

	local lawlessness = Game.system.lawlessness

	-- XXX number should be some combination of population, lawlessness,
	-- proximity to shipping lanes, etc
	local max_pirates = 6
	while max_pirates > 0 and Engine.rand:Number(1) < lawlessness do
		max_pirates = max_pirates-1

		local shipid = shiptypes[Engine.rand:Integer(1,#shiptypes)]
		local shiptype = ShipType.GetShipType(shipid)
		local default_drive = shiptype.defaultHyperdrive

		-- select a laser. this is naive - it simply chooses at random from
		-- the set of lasers that will fit, but never more than one above the
		-- player's current weapon.
		-- XXX this should use external factors (eg lawlessness) and not be
		-- dependent on the player in any way
		local max_laser_size = shiptype.capacity - EquipType.GetEquipType(default_drive).mass
		local lasers = EquipType.GetEquipTypes('LASER', function (e,et)
			return et.mass <= max_laser_size and string.sub(e,0,11) == 'PULSECANNON'
		end)
		local laser = lasers[Engine.rand:Integer(1,#lasers)]

		local ship = Space.SpawnShip(shipid, 8, 12)
		ship:AddEquip(default_drive)
		ship:AddEquip(laser)
		ship:AIKill(Game.player)
	end
end

Event.Register("onEnterSystem", onEnterSystem)
