-- Copyright © 2008-2013 Pioneer Developers. See AUTHORS.txt for details
-- Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

local police = { }
local starports
local starport

local getMyStarport = function (ship, current)
	-- check if the current system can be traded in
	starports = Space.GetBodies(function (body) return body.superType == 'STARPORT' end)

	if #starports == 0 then 
		print "found no starports"
		return nil 
	else
		print "found starports"
	end

	-- Find the nearest starport that we can land at (other than current)
	local starport, distance

	for i = 1, #starports do
		local next_starport = starports[i]
		print "looping..."
		if next_starport ~= current then
			print "--still"
			local next_distance = Game.player:DistanceTo(next_starport)
			
			local next_canland
			if ship==Game.player then
				next_canland = true
			end

			if next_canland and ((starport == nil) or (next_distance < distance)) then
				starport, distance = next_starport, next_distance
			end
		end
	end 
	return starport -- or current
end 

local spawnPolice = function()

	starport = getMyStarport(Game.player)

	if starport~=nil then

			print "spawn ships..."

		local shipdefs = build_array(filter(function (k,def) return def.tag == 'SHIP' and def.hullMass <= 150 end, pairs(ShipDef)))
		if #shipdefs == 0 then return end

		local lawlessness = Game.system.lawlessness

		-- XXX number should be some combination of population, lawlessness,
		-- proximity to shipping lanes, etc
		local max_police = 3
		while max_police > 0 do --and Engine.rand:Number(1) < lawlessness do
			max_police = max_police-1

			local shipdef = shipdefs[Engine.rand:Integer(1,#shipdefs)]
			local default_drive = shipdef.defaultHyperdrive


			local max_laser_size = shipdef.capacity - EquipDef[default_drive].mass
			local laserdefs = build_array(filter(function (k, def) return def.slot == 'LASER' and def.mass <= max_laser_size and string.sub(def.id,0,11) == 'PULSECANNON' end, pairs(EquipDef)))
			local laserdef = laserdefs[Engine.rand:Integer(1,#laserdefs)]

			local ship = Space.SpawnShipDocked(shipdef.id, starport)
			ship:AddEquip(default_drive)
			ship:AddEquip(laserdef.id)
			ship:SetLabel('POLICE')
			police[ship] = ship
			print "Police added.."
		end
	else
		print "starport was nil??"
	end


end 

local onEnterSystem = function (player)
	if not player:IsPlayer() then return end
end

local onGameStart = function ()
	onEnterSystem(Game.player)
	spawnPolice()
end

local onFrameChanged = function (ship)
	if not ship:isa('Ship') then return end
	if ship:IsPlayer() then
		if Game.player:GetFine()>=10000 then
			print "Police inc..."
			for k, v in pairs(police) do
				if v:exists() and v:GetDockedWith() then
					v:CancelAI()
					v:Undock()
				end
				if v:exists() then v:AIKill(Game.player) end
			end
		else
			print "Police gone..."
			for k, v in pairs(police) do
				if v:exists() then
					v:CancelAI()
					v:AIDockWith(starport)
				end
			end
		end

	end
end

local onAICompleted = function (ship, ai_error)
	return
end

local onShipHit = function (ship, attacker)
	onFrameChanged(Game.player)
end

local onShipAlertChanged = function (ship, alert)
	onFrameChanged(Game.player)
end

Event.Register("onAICompleted", onAICompleted)
Event.Register("onShipAlertChanged", onShipAlertChanged)
Event.Register("onShipHit", onShipHit)
Event.Register("onFrameChanged", onFrameChanged)
Event.Register("onGameStart", onGameStart)
Event.Register("onEnterSystem", onEnterSystem)
