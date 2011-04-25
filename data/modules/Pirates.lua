local ships
local candidate_ships
local lasers

local onGameStart = function ()
	ships = ShipType.GetShipTypes(ShipType.Tag.SHIP)

	candidate_ships = {}
	for n,t in pairs(ships) do
		local mass = t:GetHullMass()
		if mass >= 50 and mass <= 150 then
		    table.insert(candidate_ships, n)
	    end
	end

	lasers = EquipType.GetEquipTypes(Equip.Slot.LASER)
end

local onEnterSystem = function (player)
	if #candidate_ships == 0 then
		return
	end

	local lawlessness = Game.system:GetLawlessness()

	-- XXX number should be some combination of population, lawlessness,
	-- proximity to shipping lanes, etc
	local max_pirates = 6
	while max_pirates > 0 and Engine.rand:Number() < lawlessness do
		max_pirates = max_pirates-1

		local shiptype = candidate_ships[Engine.rand:Integer(1,#candidate_ships)]
		local default_drive = ships[shiptype]:GetDefaultHyperdrive()

		-- select a laser. this is naive - it simply chooses at random from
		-- the set of lasers that will fit, but never more than one above the
		-- player's current weapon.
		-- XXX this should use external factors (eg lawlessness) and not be
		-- dependent on the player in any way
		local max_laser_size = ships[shiptype]:GetCapacity() - EquipType.GetEquipType(default_drive):GetMass()
		local candidate_lasers = {}
		for laser,e in pairs(lasers) do
			if laser <= Game.player:GetEquip(Equip.Slot.LASER)+1 then
				local laser_size = e:GetMass()
				if laser_size <= max_laser_size then
					table.insert(candidate_lasers, laser)
				end
			end
		end
		local laser
		if #candidate_lasers == 0 then
			laser = Equip.Type.PULSECANNON_1MW
		else
			laser = candidate_lasers[Engine.rand:Integer(1,#candidate_lasers)]
		end

		ship = Space.SpawnShip(shiptype, 8, 12)
		ship:AddEquip(default_drive)
		ship:AddEquip(laser)
		ship:Destroy(Game.player)
	end
end

EventQueue.onGameStart:Connect(onGameStart)
EventQueue.onEnterSystem:Connect(onEnterSystem)
