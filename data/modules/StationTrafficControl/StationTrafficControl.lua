-- Copyright © 2008-2020 Pioneer Developers. See AUTHORS.txt for details
-- Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

local Event = require 'Event'
local Game = require 'Game'
local Engine = require 'Engine'
local Timer = require 'Timer'
local Serializer = require 'Serializer'
local Comms = require 'Comms'
local Lang = require 'Lang'

local l = Lang.GetResource("module-stationtrafficcontrol")

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

local function notifyControlledSpace ()
	if Game.player.flightState == "HYPERSPACE" then
		playerInControlledSpace = false
		return
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

end


local loaded_data

local onGameStart = function ()
	if (loaded_data) then
		playerInControlledSpace = loaded_data.playerInControlledSpace
	end
	loaded_data = nil
	Timer:CallEvery(5, notifyControlledSpace)
end


local serialize = function ()
	local data = {
		playerInControlledSpace = playerInControlledSpace,
	}
	return data
end


local unserialize = function (data)
	loaded_data = data
end

-- local onDockingClearanceGranted = function () end
-- local onDockingClearanceDenied = function () end

Event.Register("onGameStart", onGameStart)

Serializer:Register("StationTrafficControl", serialize, unserialize)
