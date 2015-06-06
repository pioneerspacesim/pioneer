-- Public Functions

-- spawn pirates
function SpawnPirate(name,risk) 
	local shipdef = shipdefs[Engine.rand:Integer(1,#shipdefs)]
	local default_drive = Equipment.hyperspace['hyperdrive_'..tostring(shipdef.hyperdriveClass)]

	local max_laser_size = shipdef.capacity - default_drive.capabilities.mass
	local laserdefs = utils.build_array(utils.filter(
		function (k,l) return l:IsValidSlot('laser_front') and l.capabilities.mass <= max_laser_size and l.l10n_key:find("PULSECANNON") end,
			pairs(Equipment.laser)
		))
	local laserdef = laserdefs[Engine.rand:Integer(1,#laserdefs)]

	name = Space.SpawnShipNear(shipdef.id, Game.player, 50, 100)
	name:SetLabel(Ship.MakeRandomLabel())
	name:AddEquip(default_drive)
	name:AddEquip(laserdef)

end
