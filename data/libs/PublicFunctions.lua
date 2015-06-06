-- Public Functions

function SpawnPirate(num_pirate_taunts) 
	local ship
		while ships > 0 do
			ships = ships-1
			if Engine.rand:Number(1) <= risk then
				local shipdef = shipdefs[Engine.rand:Integer(1,#shipdefs)]
				local default_drive = eq.hyperspace['hyperdrive_'..tostring(shipdef.hyperdriveClass)]
					
				local max_laser_size = shipdef.capacity - default_drive.capabilities.mass
				local laserdefs = utils.build_array(utils.filter(
				function (k,l) return l:IsValidSlot('laser_front') and l.capabilities.mass <= max_laser_size and l.l10n_key:find("PULSECANNON") end,
					pairs(eq.laser)
				))
				local laserdef = laserdefs[Engine.rand:Integer(1,#laserdefs)]

				ship = Space.SpawnShipNear(shipdef.id, Game.player, 50, 100)
				ship:SetLabel(Ship.MakeRandomLabel())
				ship:AddEquip(default_drive)
				ship:AddEquip(laserdef)
				ship:AddEquip(eq.misc.shield_generator, math.ceil(risk * 3))
				if Engine.rand:Number(2) <= risk then
					ship:AddEquip(eq.misc.laser_cooling_booster)
				end
				if Engine.rand:Number(3) <= risk then
					ship:AddEquip(eq.misc.shield_energy_booster)
				end
				ship:AIKill(Game.player)
				end
			end

			if ship then
				local pirate_greeting = string.interp(l["PIRATE_TAUNTS_"..Engine.rand:Integer(1,num_pirate_taunts)-1], { client = mission.client.name,})
				Comms.ImportantMessage(pirate_greeting, ship.label)
			end
end
