-- Copyright © 2008-2020 Pioneer Developers. See AUTHORS.txt for details
-- Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

local Event = require 'Event'
local Game = require 'Game'
local Engine = require 'Engine'
local Timer = require 'Timer'
local Serializer = require 'Serializer'
local Legal = require 'Legal'
local Comms = require 'Comms'
local Lang = require 'Lang'

local l = Lang.GetResource("module-crimetracking")

-- Fine at which police will launch and hunt donwn outlaw player
local maxFineTolerated = 300

-- store which station sent them out
local policeDispatched = false

-- game starts with player docked with a station
-- otherwise, serialized data already has the value
local playerInControlledSpace = true

local getNumberOfFlavours = function (str)
	-- Returns the number of flavours of the given string (assuming first flavour has suffix '_1').
	-- Taken from CargoRun.lua.
	local num = 1
	while l:get(str .. "_" .. num) do
		num = num + 1
	end
	return num - 1
end

-- check if we should dispatch police, or call them back
local function doLawAndOrder ()
	if Game.player.flightState == "HYPERSPACE" then return end

	local crimes, fine = Game.player:GetCrimeOutstanding()
	if not policeDispatched then
		if fine > maxFineTolerated and
		Game.player.flightState == "FLYING" and
		Engine.rand:Integer(0,1) > Game.system.lawlessness then
			local station = Game.player:FindNearestTo("SPACESTATION")
			-- check that station exists, since apparently empty systems can have lawlessness > 0
			if station and station.lawEnforcedRange >= station:DistanceTo(Game.player) then
				station:LaunchPolice(Game.player)
				policeDispatched = station
			end
			return
		end
	end

	-- if police are out flying about, check if they should land
	-- Note: If player docks police will land.
	if policeDispatched then
		if fine < maxFineTolerated or
		Game.player.flightState == "DOCKED" or Game.player.flightState == "DOCKING" then
			policeDispatched:LandPolice()
			policeDispatched = false
		end
	end

	if Game.player.flightState == "FLYING" then
		local station = Game.player:FindNearestTo("SPACESTATION")
		if station then
			if station:DistanceTo(Game.player) < station.lawEnforcedRange then
				if not playerInControlledSpace then
					playerInControlledSpace = true
					Comms.Message(string.interp(l["YOU_ARE_ENTERING_STATION_SPACE_" .. Engine.rand:Integer(1, getNumberOfFlavours("YOU_ARE_ENTERING_STATION_SPACE"))], {station = station.label, playerShipLabel = Game.player:GetLabel()}))
				end
			else
				if playerInControlledSpace then
					playerInControlledSpace = false
					Comms.Message(string.interp(l["YOU_ARE_LEAVING_STATION_SPACE_" .. Engine.rand:Integer(1, getNumberOfFlavours("YOU_ARE_LEAVING_STATION_SPACE"))], {station = station.label, playerShipLabel = Game.player:GetLabel()}))
				end
			end
		end
	end

	if Game.player.flightState == "HYPERSPACE" then
		playerInControlledSpace = false
	end

end


local loaded_data

local onGameStart = function ()
	if (loaded_data) then
		policeDispatched = loaded_data.policeDispatched
	end
	loaded_data = nil
	Timer:CallEvery(5, doLawAndOrder)
end


local serialize = function ()
	local data = {
		policeDispatched = policeDispatched,
		playerInControlledSpace = playerInControlledSpace,
	}
	return data
end


local unserialize = function (data)
	loaded_data = data
end


local onJettison = function(ship, cargo)
	if ship:IsPlayer() then
		if cargo.price <= 0 or not Game.system:IsCommodityLegal(cargo.name) then
			Legal:notifyOfCrime(ship,"DUMPING")
		end
	end
end


local onShipHit = function(ship, attacker)
	if attacker and attacker:IsPlayer() then
		Legal:notifyOfCrime(attacker,"PIRACY")
	end
end


local onShipDestroyed = function(ship, attacker)
	-- Note: crash issue #887, this _should_ no longer trigger crash.
	-- Also, attacker can be a body, which does not have an IsPlayer()
	if attacker and attacker:isa("Ship") and attacker:IsPlayer() then
		Legal:notifyOfCrime(attacker,"MURDER")
	end
end


local onShipFiring = function(ship)
	if ship:IsPlayer() then
		Legal:notifyOfCrime(ship,"WEAPONS_DISCHARGE")
	end
end


local onLeaveSystem = function(ship)
	if not ship:IsPlayer() then return end
	-- if we leave the system, the space station object will be invalid
	policeDispatched = nil

	if not ship:IsHyperjumpAllowed() then
		Legal:notifyOfCrime(ship,"ILLEGAL_JUMP")
	end
end


local onGameEnd = function ()
	policeDispatched = nil
end


Event.Register("onShipHit", onShipHit)
Event.Register("onShipDestroyed", onShipDestroyed)
Event.Register("onShipFiring", onShipFiring)
Event.Register("onJettison", onJettison)
Event.Register("onGameStart", onGameStart)
Event.Register("onGameEnd", onGameEnd)
Event.Register("onLeaveSystem", onLeaveSystem)


Serializer:Register("CrimeTracking", serialize, unserialize)
