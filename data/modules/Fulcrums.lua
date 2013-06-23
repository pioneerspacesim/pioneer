-- Copyright © 2008-2013 Pioneer Developers. See AUTHORS.txt for details
-- Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

local loaded
local fulcrum
local playerarrived=false

local spawnShips = function ()
	local population = Game.system.population

	if population == 0 then
		return	
	end

	local stations = Space.GetBodies(function (body) 
		return body:isa("SpaceStation") and body.type == 'STARPORT_SURFACE'
	end)
	if #stations < 2 then
		return
	end

	local station = stations[1]
	fulcrum = Space.SpawnShipParkedOffset('large_fulcrum', station)
	fulcrum:SetLabel('[--Fulcrum--]')
	fulcrum:AddEquip("ECM_ADVANCED")

	playerarrived=true
	
	return 0
end

local onEnterSystem = function (player)
	
	if player:IsPlayer() and spawnShips()~=nil then
		if fulcrum~=nil then
			local x,y,z = fulcrum:GetPos()
			y=y-200
			Game.player:SetPos(fulcrum,x,y,z)
			fulcrum:UseECM()
			Game.player:AIFlyToClose(fulcrum,500)
		end
	else
		if fulcrum~=nil and player:exists() and fulcrum:exists() then
			local x,y,z = fulcrum:GetPos()
			y=y+200
			player:SetPos(fulcrum,x,y,z)
			fulcrum:UseECM()
		end
	end
end

local onAICompleted = function (ship, ai_error)
	if not ship:IsPlayer() and playerarrived==true then return end
	playerarrived=false
end
Event.Register("onAICompleted", onAICompleted)

local onGameStart = function ()
	if loaded == nil then
		spawnShips()
	end
	loaded = nil
end

local serialize = function ()
	return playerarrived,fulcrum
end

local unserialize = function (data)
	loaded = true
	playerarrived,fulcrum = data
end

Event.Register("onEnterSystem", onEnterSystem)
Event.Register("onGameStart", onGameStart)

Serializer:Register("Fulcrum", serialize, unserialize)
