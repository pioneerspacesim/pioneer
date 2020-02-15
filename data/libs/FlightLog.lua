-- Copyright © 2008-2020 Pioneer Developers. See AUTHORS.txt for details
-- Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

--
-- Class: FlightLog
--
-- A flight log, containing the last systems and stations visited by the
-- player. Can be used by scripts to find out where the player has been
-- recently.

local Game = require 'Game'
local Event = require 'Event'
local Format = require 'Format'
local Serializer = require 'Serializer'

-- default values (private)
local FlightLogSystemQueueLength = 1000
local FlightLogStationQueueLength = 1000

-- private data - the log itself
local FlightLogSystem = {}
local FlightLogStation = {}

local FlightLog
FlightLog = {

--
-- Group: Methods
--

--
-- Method: GetSystemPaths
--
-- Returns an iterator returning a SystemPath object for each system visited
-- by the player, backwards in turn, starting with the most recent. If count
-- is specified, returns no more than that many systems.
--
-- > iterator = FlightLog.GetSystemPaths(count)
--
-- Parameters:
--
--   count - Optional. The maximum number of systems to return.
--
-- Return:
--
--   iterator - A function which will generate the paths from the log, returning
--              one each time it is called until it runs out, after which it
--              returns nil. It also returns, as secondary and tertiary values,
--              the game times at shich the system was entered and left.
--
-- Example:
--
-- Print the names and departure times of the last five systems visited by
-- the player
--
-- > for systemp,arrtime,deptime,entry in FlightLog.GetSystemPaths(5) do
-- >   print(systemp:GetStarSystem().name, Format.Date(deptime))
-- > end

	GetSystemPaths = function (maximum)
		local counter = 0
		maximum = maximum or FlightLogSystemQueueLength
		return function ()
			if counter < maximum then
				counter = counter + 1
				if FlightLogSystem[counter] then
					return FlightLogSystem[counter][1],
						   FlightLogSystem[counter][2],
						   FlightLogSystem[counter][3],
						   FlightLogSystem[counter][4]
				end
			end
			return nil, nil, nil, nil
		end
	end,


--
-- Method: UpdateSystemEntry
--
-- Update the free text field in system log.
--
-- > UpdateSystemEntry(index, entry)
--
-- Parameters:
--
--   index - Index in log, 1 being most recent (current) system
--   entry - New text string to insert instead
--
-- Example:
--
-- Replace the second most recent system record, i.e. the previously
-- visited system.
--
-- > UpdateSystemEntry(2, "At Orion's shoulder, I see attackships on fire")
--

	UpdateSystemEntry = function (index, entry)
		FlightLogSystem[index][4] = entry
	end,

--
-- Method: GetStationPaths
--
-- Returns an iterator returning a SystemPath object for each station visited
-- by the player, backwards in turn, starting with the most recent. If count
-- is specified, returns no more than that many stations.
--
-- > iterator = FlightLog.GetStationPaths(count)
--
-- Parameters:
--
--   count - Optional. The maximum number of systems to return.
--
-- Return:
--
--   iterator - A function which will generate the paths from the log, returning
--              one each time it is called until it runs out, after which it
--              returns nil. It also returns, as two additional value, the game
--              time at which the player undocked, and palyer's financial balance.
--
-- Example:
--
-- Print the names and departure times of the last five stations visited by
-- the player
--
-- > for systemp, deptime, money, entry in FlightLog.GetStationPaths(5) do
-- >   print(systemp:GetSystemBody().name, Format.Date(deptime))
-- > end

	GetStationPaths = function (maximum)
		local counter = 0
		maximum = maximum or FlightLogStationQueueLength
		return function ()
			if counter < maximum then
				counter = counter + 1
				if FlightLogStation[counter] then
					return FlightLogStation[counter][1],
						   FlightLogStation[counter][2],
						   FlightLogStation[counter][3],
						   FlightLogStation[counter][4]
				end
			end
			return nil, nil, nil, nil
		end
	end,

--
-- Method: UpdateStationEntry
--
-- Update the free text field in station log.
--
-- > UpdateStationEntry(index, entry)
--
-- Parameters:
--
--   index - Index in log, 1 being most recent station undocked from
--   entry - New text string to insert instead
--
-- Example:
--
-- Replace note for the second most recent station undocked from
--
-- > UpdateStationEntry(2, "This was a smelly station")
--

	UpdateStationEntry = function (index, entry)
		FlightLogStation[index][4] = entry
	end,

--
-- Method: GetPreviousSystemPath
--
-- Returns a SystemPath object that points to the star system where the
-- player was before jumping to this one. If none is on record (such as
-- before any hyperjumps have been made) it returns nil.
--
-- > path = FlightLog.GetPreviousSystemPath()
--
-- Return:
--
--   path - a SystemPath object
--
-- Availability:
--
--   alpha 20
--
-- Status:
--
--   experimental
--

	GetPreviousSystemPath = function ()
		if FlightLogSystem[2] then
			return FlightLogSystem[2][1]
		else return nil end
	end,

--
-- Method: GetPreviousStationPath
--
-- Returns a SystemPath object that points to the starport most recently
-- visited. If the player is currently docked, then the starport prior to
-- the present one (which might be the same one, if the player launches
-- and lands in the same port). If none is on record (such as before the
-- player has ever launched) it returns nil.
--
-- > path = FlightLog.GetPreviousStationPath()
--
-- Return:
--
--   path - a SystemPath object
--
-- Availability:
--
--   alpha 20
--
-- Status:
--
--   experimental
--

	GetPreviousStationPath = function ()
		if FlightLogStation[1] then
			return FlightLogStation[1][1]
		else return nil end
	end,

}

-- LOGGING

-- onLeaveSystem
local AddSystemDepartureToLog = function (ship)
	if not ship:IsPlayer() then return end
	FlightLogSystem[1][3] = Game.time
	while #FlightLogSystem > FlightLogSystemQueueLength do
		table.remove(FlightLogSystem,FlightLogSystemQueueLength + 1)
	end
end

-- onEnterSystem
local AddSystemArrivalToLog = function (ship)
	if not ship:IsPlayer() then return end
	table.insert(FlightLogSystem,1,{Game.system.path,Game.time,nil,""})
	while #FlightLogSystem > FlightLogSystemQueueLength do
		table.remove(FlightLogSystem,FlightLogSystemQueueLength + 1)
	end
end

-- onShipUndocked
local AddStationToLog = function (ship, station)
	if not ship:IsPlayer() then return end
	table.insert(FlightLogStation,1,{station.path, Game.time, Game.player:GetMoney(), ""})
	while #FlightLogStation > FlightLogStationQueueLength do
		table.remove(FlightLogStation,FlightLogStationQueueLength + 1)
	end
end

-- LOADING AND SAVING

local loaded_data

local onGameStart = function ()
	if loaded_data then
		FlightLogSystem = loaded_data.System
		FlightLogStation = loaded_data.Station
	else
		table.insert(FlightLogSystem,1,{Game.system.path,nil,nil,""})
	end
	loaded_data = nil
end

local onGameEnd = function ()
	FlightLogSystem = {}
	FlightLogStation = {}
	FlightLogCustom = {}
end

local serialize = function ()
	return { System = FlightLogSystem, Station = FlightLogStation }
end

local unserialize = function (data)
	loaded_data = data
end

Event.Register("onEnterSystem", AddSystemArrivalToLog)
Event.Register("onLeaveSystem", AddSystemDepartureToLog)
Event.Register("onShipUndocked", AddStationToLog)
Event.Register("onGameStart", onGameStart)
Event.Register("onGameEnd", onGameEnd)
Serializer:Register("FlightLog", serialize, unserialize)

return FlightLog
