-- Copyright © 2008-2024 Pioneer Developers. See AUTHORS.txt for details
-- Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

local Event = require 'Event'
local Game = require 'Game'
local Engine = require 'Engine'
local Timer = require 'Timer'
local Serializer = require 'Serializer'
local Comms = require 'Comms'
local Space = require 'Space'
local Lang = require 'Lang'

local l = Lang.GetResource("module-stationtrafficcontrol")
local ui = require 'pigui'

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

local enteringFlavors = getNumberOfFlavours("YOU_ARE_ENTERING_STATION_SPACE")
local leavingFlavors = getNumberOfFlavours("YOU_ARE_LEAVING_STATION_SPACE")
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
					local message = l["YOU_ARE_ENTERING_STATION_SPACE_" .. Engine.rand:Integer(1, enteringFlavors)]
					Comms.Message(string.interp(message, { station = station.label, playerShipLabel = Game.player:GetLabel() }))
				end
			else
				if playerInControlledSpace then
					playerInControlledSpace = false
					local message = l["YOU_ARE_LEAVING_STATION_SPACE_" .. Engine.rand:Integer(1, leavingFlavors)]
					Comms.Message(string.interp(message, { station = station.label, playerShipLabel = Game.player:GetLabel() }))
				end
			end
		end
	end

end

local function getAtmoState(station)
	local parent_systemBody = station.path:GetSystemBody().parent
	local parent_frameBody = Space.GetBody(parent_systemBody.index)
	local gravity, pressure = parent_systemBody.gravity, parent_systemBody.hasAtmosphere and parent_frameBody:GetAtmosphericState(station)
	return gravity / 9.8, parent_systemBody.hasAtmosphere, pressure
end

local function onDockingClearanceGranted(station, ship)
	if ship ~= Game.player then return end
	local bayNum = station:GetAssignedBayNumber(ship)
	local nearby = station:GetNearbyTraffic(50000) -- 50km
	local gravity, hasAtmo, pressure = getAtmoState(station)

	Comms.Message(l.CLEARANCE_GRANTED_BAY_N:interp({ bay = station:GetAssignedBayNumber(ship) + 1 }), station.label)

	local conditionMessage = ""
	if station.type == "STARPORT_SURFACE" then
		if hasAtmo and pressure > 0.01 then
			conditionMessage = l.STATION_GRAV_PRESS:interp({
				grav = ui.Format.Gravity(gravity),
				press = ui.Format.Pressure(pressure)})
		else
			conditionMessage = l.STATION_GRAV:interp({ grav = ui.Format.Gravity(gravity) })
		end
	end

	if nearby > 0 then conditionMessage = conditionMessage .. " " .. l.WATCH_FOR_TRAFFIC_ON_APPROACH end
	if #conditionMessage > 0 then Comms.Message(conditionMessage, station.label) end
end

local function onDockingClearanceDenied(station, ship, reason)
	if ship ~= Game.player then return end
	print(reason)
	if reason == "ClearanceAlreadyGranted" then
		Comms.Message(l.CLEARANCE_ALREADY_GRANTED_BAY_N:interp({ bay = station:GetAssignedBayNumber(ship) + 1 }), station.label)
	elseif reason == "TooFarFromStation" then
		Comms.Message(l.CLEARANCE_DENIED_TOO_FAR, station.label)
	else
		Comms.Message(l.CLEARANCE_DENIED_NO_BAYS, station.label)
	end
end

local function onDockingClearanceExpired(station, ship)
	if ship ~= Game.player then return end
	Comms.ImportantMessage(l.DOCKING_CLEARANCE_EXPIRED, station.label)
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

Event.Register("onGameStart", onGameStart)
Event.Register("onDockingClearanceGranted", onDockingClearanceGranted)
Event.Register("onDockingClearanceDenied", onDockingClearanceDenied)
Event.Register("onDockingClearanceExpired", onDockingClearanceExpired)

Serializer:Register("StationTrafficControl", serialize, unserialize)
