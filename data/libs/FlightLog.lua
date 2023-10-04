-- Copyright © 2008-2023 Pioneer Developers. See AUTHORS.txt for details
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

local utils = require 'utils'

local Character = require 'Character'


-- required for formatting / localisation
local ui = require 'pigui'
local Lang = require 'Lang'
local l = Lang.GetResource("ui-core")
-- end of formating / localisation stuff

-- default values (private)
local FlightLogSystemQueueLength = 1000
local FlightLogStationQueueLength = 1000

-- private data - the log itself
local FlightLogSystem = {}
local FlightLogStation = {}
local FlightLogCustom = {}

-- a generic log entry:
LogEntry = utils.class("FlightLog.LogEntry.Base")

function LogEntry:GetType()
	local full_type = self.Class().meta.class
	return string.sub( full_type, #"FlightLog.LogEntry."+1, #full_type )
end

-- return true if this has a modifiable entry field
function LogEntry:HasEntry()
	return true
end


-- return true if this has a Delete() method
function LogEntry:SupportsDelete()
	return false
end

function LogEntry:Constructor( index, sort_date, entry )
	-- the index of the entry from the list which it came
	self.index = index
	-- a date that can be used to sort entries on TODO: remove this
	self.sort_date = sort_date
	-- the entry text associated with this log entry
	self.entry = entry
	self.type = self:GetType()
end

-- return the name for this log entry type
function LogEntry:GetLocalizedName()
	return "Error"
end

-- return an array of key value pairs, the key being localized and the
-- value being formatted appropriately.
function LogEntry:GetDataPairs()
	return { "ERROR", "This should never be seen" }
end

function LogEntry:UpdateEntry( entry )
	self.entry = entry
end

-- convenience helper function
-- Sometimes date is empty, e.g. departure date prior to departure
-- TODO: maybe not return this at all then!
function LogEntry.formatDate(date)
	return date and Format.Date(date) or nil
end

-- Based on flight state, compose a reasonable string for location
function LogEntry.composeLocationString(location)
	return string.interp(l["FLIGHTLOG_"..location[1]],
		{ primary_info = location[2],
			secondary_info = location[3] or "",
			tertiary_info = location[4] or "",})
end

CurrentStatusLogEntry = utils.class("FilghtLog.LogEntry.CurrentStatus", LogEntry )

function CurrentStatusLogEntry:HasEntry()
	return false
end


function CurrentStatusLogEntry:Constructor()
	LogEntry.Constructor( self, 0, Game.time, "" )
end

function CurrentStatusLogEntry:GetLocalizedName()
	return l.PERSONAL_INFORMATION;
end

-- return an array of key value pairs, the key being localized and the
-- value being formatted appropriately.
function CurrentStatusLogEntry:GetDataPairs()

    local player = Character.persistent.player

	return {
		{ l.NAME_PERSON, player.name },
		-- TODO: localize
		{ "Title", player.title },
		{ l.RATING, l[player:GetCombatRating()] },
		{ l.KILLS,  string.format('%d',player.killcount) }
	}
end

SystemLogEntry = utils.class("FlightLog.LogEntry.System", LogEntry)

function SystemLogEntry:Constructor( index, systemp, arrtime, deptime, entry )

	if nil == arrtime then
		sort_date = deptime
	else
		sort_date = arrtime
	end

	LogEntry.Constructor( self, index, sort_date, entry )

	self.systemp = systemp
	self.arrtime = arrtime
	self.deptime = deptime

end

function SystemLogEntry:GetLocalizedName()
	return l.LOG_SYSTEM;
end

-- return an array of key value pairs, the key being localized and the
-- value being formatted appropriately.
function SystemLogEntry:GetDataPairs()
	return {
		{ l.ARRIVAL_DATE, self.formatDate(self.arrtime) },
		{ l.DEPARTURE_DATE, self.formatDate(self.deptime) },
		{ l.IN_SYSTEM, ui.Format.SystemPath(self.systemp) },
		{ l.ALLEGIANCE, self.systemp:GetStarSystem().faction.name }
	}
end

function SystemLogEntry:UpdateEntry( entry )
	LogEntry.UpdateEntry( self, entry )
	FlightLogSystem[self.index][4] = entry
end

-- a custom log entry:
CustomLogEntry = utils.class("FlightLog.LogEntry.Custom", LogEntry)

function CustomLogEntry:Constructor( index, systemp, time, money, location, entry  )
	LogEntry.Constructor( self, index, time, entry )

	self.systemp = systemp
	self.time = time
	self.money = money
	self.location = location
end

function CustomLogEntry:GetLocalizedName()
	return l.LOG_CUSTOM;
end

-- return an array of key value pairs, the key being localized and the
-- value being formatted appropriately.
function CustomLogEntry:GetDataPairs()
	return {
		{ l.DATE, self.formatDate(self.time) },
		{ l.LOCATION, self.composeLocationString(self.location) },
		{ l.IN_SYSTEM, ui.Format.SystemPath(self.systemp) },
		{ l.ALLEGIANCE, self.systemp:GetStarSystem().faction.name },
		{ l.CASH, Format.Money(self.money) }
	}
end


function CustomLogEntry:UpdateEntry( entry )
	LogEntry.UpdateEntry( self, entry )
	FlightLogCustom[self.index][5] = entry
end

-- return true if this has a Delete() method
function CustomLogEntry:SupportsDelete()
	return true
end

function CustomLogEntry:Delete()
	table.remove(FlightLogCustom, index)

	-- TODO: now all the indexes are wrong.
	-- we should do away with them all!
end


-- a station log entry:
StationLogEntry = utils.class("FlightLog.LogEntry.Station", LogEntry)

function StationLogEntry:Constructor( index, systemp, deptime, money, entry   )
	LogEntry.Constructor( self, index, deptime, entry )

	self.systemp = systemp
	self.deptime = deptime
	self.money = money
end

function StationLogEntry:GetLocalizedName()
	return l.LOG_STATION;
end

-- return an array of key value pairs, the key being localized and the
-- value being formatted appropriately.
function StationLogEntry:GetDataPairs()

	local station_type = "FLIGHTLOG_" .. self.systemp:GetSystemBody().type


	return {
		{ l.DATE, self.formatDate(self.deptime) },
		{ l.STATION, string.interp(l[station_type],
		{	primary_info = self.systemp:GetSystemBody().name,
			secondary_info = self.systemp:GetSystemBody().parent.name }) },
--		{ l.LOCATION, self.systemp:GetSystemBody().parent.name },
		{ l.IN_SYSTEM, ui.Format.SystemPath(self.systemp) },
		{ l.ALLEGIANCE, self.systemp:GetStarSystem().faction.name },
		{ l.CASH, Format.Money(self.money) },
	}
end


function StationLogEntry:UpdateEntry( entry )
	LogEntry.UpdateEntry( self, entry )
	FlightLogStation[self.index][4] = entry
end

local FlightLog
FlightLog = {

--
-- Group: Methods
--
--
-- Method: GetSystemPaths
--
-- Returns an iterator returning a SystemLogEntry object for each system visited
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
-- > for entry in FlightLog.GetSystemPaths(5) do
-- >   print(entry.systemp:GetStarSystem().name, Format.Date(entry.deptime))
-- > end

	GetSystemPaths = function (maximum)
		local counter = 0
		maximum = maximum or FlightLogSystemQueueLength
		return function ()
			if counter < maximum then
				counter = counter + 1
				if FlightLogSystem[counter] then

				local entry = SystemLogEntry.New(
					counter,
					FlightLogSystem[counter][1],
					FlightLogSystem[counter][2],
					FlightLogSystem[counter][3],
					FlightLogSystem[counter][4] )

				return entry

				end
			end
			return nil
		end
	end,

--
-- Method: GetStationPaths
--
-- Returns an iterator returning a StationLogEntry object for each station visited
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
--   iterator - A function which will generate the StationLogEntry from the log,
--              returning one each time it is called until it runs out, after
--              which it returns nil. 
--
-- Example:
--
-- Print the names and arrival times of the last five stations visited by
-- the player
--
-- > for entry in FlightLog.GetStationPaths(5) do
-- >   print(entry.systemp:GetSystemBody().name, Format.Date(entry.deptime))
-- > end

	GetStationPaths = function (maximum)
		local counter = 0
		maximum = maximum or FlightLogStationQueueLength
		return function ()
			if counter < maximum then
				counter = counter + 1
				if FlightLogStation[counter] then
					return StationLogEntry.New(
						counter,
						FlightLogStation[counter][1],
						FlightLogStation[counter][2],
						FlightLogStation[counter][3],
						FlightLogStation[counter][4] )
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
--   index - Index in log, 1 being most recent station docked with
--   entry - New text string to insert instead
--
-- Example:
--
-- Replace note for the second most recent station docked with
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



--
-- Method: GetCustomEntry
--
-- Returns an iterator returning a CustomLogEntry, backwards in turn,
-- starting with the most recent for each log entry the player has made.
-- If count is specified, returns no more than that many entries.
--
-- > iterator = FlightLog.GetCustomEntry(count)
--
-- Parameters:
--
--   count - Optional. The maximum number of entries to return.
--
-- Return:
--
--   iterator - A function which will generate the entries from the
--              log, returning one each time it is called until it
--              runs out, after which it returns nil. Each entry is
--				a CustomLogEntry
--
-- Example:
--
-- > for entry in FlightLog.GetCustomEntry(5) do
-- >   print(entry.location[1], entry.location[2], Format.Date(entry.deptime))
-- > end
--

	GetCustomEntry = function (maximum)
		local counter = 0
		maximum = maximum or #FlightLogCustom
		return function ()
			if counter < maximum then
				counter = counter + 1
				if FlightLogCustom[counter] then
					return CustomLogEntry.New(
						counter,
						FlightLogCustom[counter][1], --path
						FlightLogCustom[counter][2], --time
						FlightLogCustom[counter][3], --money
						FlightLogCustom[counter][4], --location
						FlightLogCustom[counter][5]  --manual entry
					)
				end
			end
			return
		end
	end,

--
-- Method: UpdateCustomEntry
--
-- Update the free text field with new entry. Allows the player to
-- change the original text entry.
--
-- > FlightLog.GetCustomEntry(index, entry)
--
-- Parameters:
--
--   index - Position in log, 1 being most recent
--   entry - String of new text to replace the original with
--
-- Example:
--
-- > FlightLog.UpdateCustomEntry(2, "Earth is an overrated spot")
--

	UpdateCustomEntry = function (index, entry)
		FlightLogCustom[index][5] = entry
	end,

--
-- Method: DeleteCustomEntry
--
-- Remove an entry.
--
-- > FlightLog.DeleteCustomEntry(index)
--
-- Parameters:
--
--   index - Position in log to remove, 1 being most recent
--

	DeleteCustomEntry = function (index)
		table.remove(FlightLogCustom, index)
	end,

--
-- Method: MakeCustomEntry
--
-- Create a custom entry. A set of information is automatically
-- compiled, in a header.
--
-- > FlightLog.MakeCustomEntry(text)
--
-- Header:
--
--   path - System path, pointing to player's current sytem
--   time - Game date
--   money - Financial balance at time of record creation
--   location - Array, with two strings: flight state, and relevant additional string
--   manual entry - Free text string
--
-- Parameters:
--
--   text - Text to accompany the log
--

	MakeCustomEntry = function (text)
		text = text or ""
		local location = ""
		local state = Game.player:GetFlightState()
		local path = ""

		if state == "DOCKED" then
			local station = Game.player:GetDockedWith()
			local parent_body = station.path:GetSystemBody().parent.name
			location = {station.type, station.label, parent_body}
			path = Game.system.path
		elseif state == "DOCKING" or state == "UNDOCKING" then
			location = {state, Game.player:FindNearestTo("SPACESTATION").label}
			path = Game.system.path
		elseif state == "FLYING" then
			if Game.player.frameBody then
				location = {state, Game.player.frameBody.label}
			else
				location = {state, Game.system.name} -- if orbiting a system barycenter, there will be no frame object
			end
			path = Game.system.path
		elseif state == "LANDED" then
			path = Game.system.path
			local alt, vspd, lat, long = Game.player:GetGPS()
			if not (lat and long) then
				lat, long = "nil", "nil"
			end
			location = {state, Game.player:FindNearestTo("PLANET").label, lat, long}
		elseif state == "JUMPING" or state == "HYPERSPACE" then
			--if in hyperspace, there's no Game.system
			local spath, sysname = Game.player:GetHyperspaceDestination()
			path = spath
			location = {state, sysname}
		end

		table.insert(FlightLogCustom,1,
			{path, Game.time, Game.player:GetMoney(), location, text})
	end,

}

--
-- Method: GetLogEntries
--
-- Parameters:
--
--   types - An array of the types we want to fetch, nil if all of them
--   maximum - the maximum number of results to return
-- Return:
--
--   iterator - A function which will generate the entries from the
--              log, returning one each time it is called until it
--              runs out, after which it returns nil. Each entry is
--				a child class of LogEntry
--
-- Example:
--
-- > for entry in FlightLog.GetLogEntries( { "Custom", "System", "Station" ) do
-- >   print( entry.GetType(), entry.entry )
-- > end
function FlightLog:GetLogEntries(types, maximum)

	-- TODO: actually just store a list of all of them as they are at startup
	local type_set = utils.set.New(types)

	local all_entries = {}

	if nil == types or type_set:contains( 'Custom' ) then			
		for entry in self.GetCustomEntry() do
			table.insert( all_entries, entry )
		end
	end

	if nil == types or type_set:contains( 'Station' ) then			
		for entry in self.GetStationPaths() do
			table.insert( all_entries, entry )
		end
	end

	if nil == types or type_set:contains( 'System' ) then			
		for entry in self.GetSystemPaths() do
			table.insert( all_entries, entry )
		end
	end



	local function sortf( a, b )
		return a.sort_date > b.sort_date
	end



	table.sort( all_entries, sortf )

	-- note regardless of sort order, current status always comes first.
	local currentStatus = nil
	if nil == types or type_set:contains( "CurrentStatus" ) then
		currentStatus = CurrentStatusLogEntry.New()
	end

	local counter = 0
	maximum = maximum or #all_entries
	return function ()
		if currentStatus then
			t = currentStatus
			currentStatus = nil
			return t
		end
		if counter < maximum then
			counter = counter + 1
			return all_entries[counter]
		end
		return nil
	end
end


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

-- onShipDocked
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
	if loaded_data and loaded_data.Version >= 1 then
		FlightLogSystem = loaded_data.System
		FlightLogStation = loaded_data.Station
		FlightLogCustom = loaded_data.Custom
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
	return { System = FlightLogSystem,
			 Station = FlightLogStation,
			 Custom = FlightLogCustom,
			 Version = 1 -- version for backwards compatibility
	}
end

local unserialize = function (data)
	loaded_data = data
end

Event.Register("onEnterSystem", AddSystemArrivalToLog)
Event.Register("onLeaveSystem", AddSystemDepartureToLog)
Event.Register("onShipDocked", AddStationToLog)
Event.Register("onGameStart", onGameStart)
Event.Register("onGameEnd", onGameEnd)
Serializer:Register("FlightLog", serialize, unserialize)

return FlightLog
