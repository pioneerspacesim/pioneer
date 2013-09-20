-- Copyright © 2008-2013 Pioneer Developers. See AUTHORS.txt for details
-- Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

local Engine = import("Engine")
local Game = import("Game")
local Space = import("Space")
local Comms = import("Comms")
local Timer = import("Timer")
local Event = import("Event")
local Serializer = import("Serializer")
local ShipDef = import("ShipDef")
local utils = import("utils")

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
		fulcrum=nil
		playerarrived=true
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
	return {playerarrived,fulcrum}
end

local unserialize = function (data)
	loaded = true
	playerarrived=data[1]
	fulcrum=data[2]
end

local onGameEnd = function ()
	-- drop the references for our data so Lua can free them
	-- and so we can start fresh if the player starts another game
	loaded,fulcrum,playerarrived=nil,nil,nil
end

Event.Register("onGameEnd", onGameEnd)
Event.Register("onEnterSystem", onEnterSystem)
Event.Register("onGameStart", onGameStart)
Serializer:Register("Fulcrum", serialize, unserialize)
