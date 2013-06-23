-- Copyright © 2008-2013 Pioneer Developers. See AUTHORS.txt for details
-- Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

local police = { }
local starports
local starport
local warn=false

local getMyStarport = function (ship, current)
	-- check if the current system can be traded in
	starports = Space.GetBodies(function (body) return body.superType == 'STARPORT' end)

	if #starports == 0 then 
		return nil 
	end

	-- Find the nearest starport that we can land at (other than current)
	local starport, distance

	for i = 1, #starports do
		local next_starport = starports[i]
		if next_starport ~= current then
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

	for k,v in pairs(police) do 
		if v.ship~=nil and v.ship:exists() then return end
	end

	if starport~=nil then

		if Game.player:DistanceTo(starport)>400000 then return end

		-- XXX number should be some combination of population, lawlessness,
		-- proximity to shipping lanes, etc
		local max_police = 3
		while max_police > 0 do --and Engine.rand:Number(1) < lawlessness do
			max_police = max_police-1

			local ship = Space.SpawnShipDocked("kanara", starport)
			ship:AddEquip('PULSECANNON_DUAL_1MW')
			ship:AddEquip('LASER_COOLING_BOOSTER')
			ship:AddEquip('ATMOSPHERIC_SHIELDING')
			ship:SetLabel('POLICE')
			police[ship] = {
				ship	= ship,
			}
		end
	end


end 

local deletePolice = function (player)
	if not player:IsPlayer() then return end
	for k, v in pairs(police) do
		if v.ship~=nil and v.ship:exists() then
			v.ship:CancelAI()
			v.ship:Explode()
			police[v.ship] = nil
		end
		warn=false
	end
end

local onEnterSystem = function (player)
	if not player:IsPlayer() then return end
	deletePolice(player)
end

local onGameStart = function ()
	starport = getMyStarport(Game.player)
	spawnPolice()
end

local doLawAndOrder = function (ship)
	if not ship:isa('Ship') then return end
	if ship:IsPlayer() then

		starport = getMyStarport(Game.player)
		if Game.player:DistanceTo(starport)<400000 then
			spawnPolice()
		else
			deletePolice(Game.player)
			return
		end

		if Game.player:GetFine()>=10000 then
			for k, v in pairs(police) do
				if v.ship~=nil and v.ship:exists() and v.ship:GetDockedWith() then
					v.ship:CancelAI()
					v.ship:Undock()
				end
				if v.ship~=nil and v.ship:exists() then
					v.ship:AIFlyTo(Game.player)
				end
				if not warn then
					Comms.ImportantMessage(Game.player.label..', Please pay outstanding fines immediately, or we will be forced to open fire!', v.ship.label)
					warn=true
				end
				Timer:CallAt(Game.time+20, function ()
					if v.ship~=nil and v.ship:exists() and Game.player:GetFine()>=10000 then v.ship:AIKill(Game.player) end
				end)
			end
		else
			print "Police idle..."
			for k, v in pairs(police) do
				if v.ship~=nil and v.ship:exists() then
					v.ship:CancelAI()
					v.ship:AIDockWith(starport)
					warn=false
				end
			end
		end

	end
end

local onFrameChanged = function (ship)
	if not ship:isa('Ship') then return end
	if ship:IsPlayer() then
		doLawAndOrder(ship)
	end
end

local onShipHit = function (ship, attacker)
	if not ship:IsPlayer() then return end
	doLawAndOrder(Game.player)
end

local onShipAlertChanged = function (ship, alert)
	doLawAndOrder(Game.player)
end

Event.Register("onShipAlertChanged", onShipAlertChanged)
Event.Register("onShipHit", onShipHit)
Event.Register("onFrameChanged", onFrameChanged)
Event.Register("onGameStart", onGameStart)
Event.Register("onEnterSystem", onEnterSystem)
