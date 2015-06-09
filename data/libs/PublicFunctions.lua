-- Public Functions

-- spawn pirates
function SpawnPirate(risk) 
	local ship
	local default_drive = Equipment.hyperspace['hyperdrive_'..tostring(shipdef.hyperdriveClass)]

	local max_laser_size = shipdef.capacity - default_drive.capabilities.mass
	local laserdefs = utils.build_array(utils.filter(
		function (k,l) return l:IsValidSlot('laser_front') and l.capabilities.mass <= max_laser_size and l.l10n_key:find("PULSECANNON") end,
			pairs(Equipment.laser)
		))
	local laserdef = laserdefs[Engine.rand:Integer(1,#laserdefs)]

	ship = Space.SpawnShipNear(shipdef.id, Game.player, 50, 100)
	ship:SetLabel(Ship.MakeRandomLabel())
	ship:AddEquip(default_drive)
	ship:AddEquip(laserdef)
	return ship

end
