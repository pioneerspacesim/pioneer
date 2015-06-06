-- Public Functions

-- spawn pirates
function SpawnPirate(name,risk) 
		if Engine.rand:Number(1) <= risk then
			local shipdef = shipdefs[Engine.rand:Integer(1,#shipdefs)]
			local default_drive = eq.hyperspace['hyperdrive_'..tostring(shipdef.hyperdriveClass)]
					
			local max_laser_size = shipdef.capacity - default_drive.capabilities.mass
			local laserdefs = utils.build_array(utils.filter(
			function (k,l) return l:IsValidSlot('laser_front') and l.capabilities.mass <= max_laser_size and l.l10n_key:find("PULSECANNON") end,
				pairs(eq.laser)
			))
			local laserdef = laserdefs[Engine.rand:Integer(1,#laserdefs)]

			name = Space.SpawnShipNear(shipdef.id, Game.player, 50, 100)
			name:SetLabel(Ship.MakeRandomLabel())
			name:AddEquip(default_drive)
			name:AddEquip(laserdef)
			name:AddEquip(eq.misc.shield_generator, math.ceil(risk * 3))
			if Engine.rand:Number(2) <= risk then
				name:AddEquip(eq.misc.laser_cooling_booster)
			end
			if Engine.rand:Number(3) <= risk then
				name:AddEquip(eq.misc.shield_energy_booster)
			end
			name:AIKill(Game.player)
			end

end
