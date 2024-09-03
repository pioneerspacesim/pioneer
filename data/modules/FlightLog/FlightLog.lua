-- Copyright © 2008-2024 Pioneer Developers. See AUTHORS.txt for details
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

local FlightLogEntry = require 'modules.FlightLog.FlightLogEntries'

-- default values (private)
---@type integer
local MaxTotalNonCustomElements = 3000

-- private data - the log itself
---@type FlightLogEntry.Base[]
local FlightLogData = {}

local FlightLog = {}

---How many default (so without custom elements) entries are there?
---@type integer
local NonCustomElementCount = 0

---@praram count integer The amount to alter the count of default elements by
local function AdjustNonCustomElementCount( count )
	NonCustomElementCount = NonCustomElementCount + count
end

FlightLogEntry.Base.non_custom_count_change = AdjustNonCustomElementCount

--- If there are two system events back to back, starting at first_index
--- entering and leaving the same system, it will put them together
--- as a single system event
---
---@param first_index integer The index of the first element in the array (so the latest event) to collapse
---@return boolean true if the two were collapesed.
local function ConsiderCollapseSystemEventPair( first_index )

	if #FlightLogData < 2 then return false end

	local second = FlightLogData[first_index]
	local first = FlightLogData[first_index+1]

	if ( second:IsCustom() ) then return false end
	if ( second:GetType() ~= "System" ) then return false end
	---@cast second SystemLogEntry

	-- is the latest one actually an arrival event, or already collapsed.
	if ( second.arrtime ~= nil ) then return fale end

	if ( first:IsCustom() ) then return false end
	if ( first:GetType() ~= "System" ) then return false end
	---@cast first SystemLogEntry

	-- is the first one actually a departure event or already collapsed
	if ( first.deptime ~= nil ) then return false end

	if ( first.systemp ~= second.systemp ) then return false end

	second.arrtime = first.arrtime
	table.remove( FlightLogData, first_index+1 )

	AdjustNonCustomElementCount( -1 )

	return true
end


-- This will run through the array of events and if there are two system events
-- back to back, entering and leaving the same system, it will put them together
-- as a single system event
local function CollapseSystemEvents()
	for i = #FlightLogData-1, 1, -1 do
		ConsiderCollapseSystemEventPair( i )
	end
end

-- This will run through the array of events and remove any non custom ones
-- if we have exceeded our maximum size, until that maximum size is reattained.
local function TrimLogSize()
	if NonCustomElementCount > MaxTotalNonCustomElements then
		CollapseSystemEvents()

		if NonCustomElementCount > MaxTotalNonCustomElements then
			for i = #FlightLogData, 1, -1 do
				local v = FlightLogData[i]
				if not v:IsCustom() then
					table.remove( FlightLogData, i )
					AdjustNonCustomElementCount( -1 )
					if i > 1 then
						ConsiderCollapseSystemEventPair( i-1 )
					end

					if NonCustomElementCount <= MaxTotalNonCustomElements then
						return
					end
				end
			end
		end
	end
end

--
-- Group: Methods
--
-- Method: AddEntry
--
-- Adds an entry to the flightlog
--
-- Parameters:
--   entry - The entry to add to the list
---@param entry FlightLogEntry.Base
function FlightLog.AddEntry(entry)
	table.insert(FlightLogData, 1, entry )

	if not entry:IsCustom() then
		AdjustNonCustomElementCount( 1 )
	end

	if not ConsiderCollapseSystemEventPair(1) then
		TrimLogSize()
	end
end

-- Method: RemoveEntry
--
-- Removes an entry from the flightlog
--
-- Parameters:
--   entry - The entry to remove from the list
---@param entry FlightLogEntry.Base
function FlightLog:RemoveEntry(entry)
	if not entry:IsCustom() then
		AdjustNonCustomElementCount( -1 )
	end
	local index = utils.remove_elem( FlightLogData, entry )
	if index > 1 then
		ConsiderCollapseSystemEventPair( index-1 )
	end
end

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
---@param text string?
function FlightLog.MakeCustomEntry(text)
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

	FlightLog.AddEntry( FlightLogEntry.Custom.New( path, Game.time, Game.player:GetMoney(), location, text ) )
end

--
-- Method: InsertCustomEntry
--
-- Insert an element into custom flight log with all required fields
--
-- > FlightLog.InsertCustomEntry(entry)
--
-- Parameters:
--
--   entry - table
--
-- Entry:
--
--   path - System path
--   time - Game date
--   money - Financial balance at time of record creation
--   location - Array, with two strings: flight state, and relevant additional string
--   entry - Free text string
--
---@param entry table
function FlightLog.InsertCustomEntry(entry)
	if entry.path and entry.time and entry.money and entry.location and entry.entry then
		FlightLog.AddEntry( FlightLogEntry.Custom.New( entry.path, entry.time, entry.money, entry.location, entry.entry ) )
		return true
	else
		return false
	end
end

--
-- Method: InsertSystemEntry
--
-- Insert an element into system flight log with all required fields
--
-- > FlightLog.InsertSystemEntry(entry)
--
-- Parameters:
--
--   entry - table
--
-- Entry:
--
--   path - System path, pointing to player's current system
--   arrtime - Game date, arrival
--   deptime - Game date, departure (optional)
--   entry - Free text string (optional)
--
---@param entry table
function FlightLog.InsertSystemEntry(entry)
	if entry.path and (entry.arrtime or entry.deptime) then
		FlightLog.AddEntry( FlightLogEntry.System.New( entry.path, entry.arrtime, entry.deptime, entry.entry or "" ) )
		return true
	else
		return false
	end
end

--
-- Method: InsertStationEntry
--
-- Insert an element into station flight log with all required fields
--
-- > FlightLog.InsertStationEntry(entry)
--
-- Parameters:
--
--   entry - table
--
-- Entry:
--
--   path - System path
--   deptime - Game date, _arrival_
--   money - Financial balance at time of record creation
--   entry - Free text string (optional)
--
---@param entry table
function FlightLog.InsertStationEntry(entry)
	if entry.path and entry.deptime and entry.money then
		FlightLog.AddEntry( FlightLogEntry.Station.New( entry.path, entry.deptime, entry.money, entry.entry or "" ) )
		return true
	else
		return false
	end
end

-- Method: GetLogEntries
--
-- Example:
--
-- > for entry in FlightLog.GetLogEntries( { "Custom", "System", "Station" ) do
-- >   print( entry.GetType(), entry.entry )
-- > end
--
-- Parameters:
--
--    types - A table where keys are log types to include and the value is a boolean, set to true to include that type
--    maximum  - An optional integer, with the maximum number of entries to include
--    earliest_first - An optional boolean to say if the log should be ordered in chronological order or reverse chronological order
--
-- Returns:
--
--    An iterator function that when called repeatedly returns the next entry or nil when complete
--
---@param types table<string,boolean>?
---@param maximum integer?
---@param earliest_first boolean?
---
---@return fun():FlightLogEntry.Base
function FlightLog:GetLogEntries(types, maximum, earliest_first)

	local counter = 0
	maximum = maximum or #FlightLogData
	return function ()
		while counter < maximum do
			counter = counter + 1

			local v
			if earliest_first then
				v = FlightLogData[(#FlightLogData+1) - counter]
			else
				v = FlightLogData[counter]
			end
			-- TODO: Can we map the types to serialization indexes and check these
			-- as they may be faster than the  manipulation comapare stuff.
			if nil == types or types[ v:GetType() ] then
				return v
			end
		end
		return nil
	end
end

-- LOGGING

-- onLeaveSystem
local AddSystemDepartureToLog = function (ship)
	if not ship:IsPlayer() then return end

	FlightLog.AddEntry( FlightLogEntry.System.New( Game.system.path, nil, Game.time, nil ) )
end

-- onEnterSystem
local AddSystemArrivalToLog = function (ship)
	if not ship:IsPlayer() then return end

	FlightLog.AddEntry( FlightLogEntry.System.New( Game.system.path, Game.time, nil, nil ) )
end

-- onShipDocked
local AddStationToLog = function (ship, station)
	if not ship:IsPlayer() then return end

	FlightLog.AddEntry( FlightLogEntry.Station.New( station.path, Game.time, Game.player:GetMoney(), nil ) )
end

function FlightLog.OrganizeEntries()

	local function sortf( a, b )
		return a.sort_date > b.sort_date
	end

	table.sort( FlightLogData, sortf )

	CollapseSystemEvents()
end

-- LOADING AND SAVING

local loaded_data

local onGameStart = function ()

	if loaded_data then
		FlightLogData = loaded_data.Data
	end

	NonCustomElementCount = 0
	for _, e in pairs(FlightLogData) do
		if not e:IsCustom() then
			AdjustNonCustomElementCount( 1 )
		end
	end

	loaded_data = nil
end

local onGameEnd = function ()
	FlightLogData = {}
	NonCustomElementCount = 0
end

local serialize = function ()
	return {
		Data = FlightLogData,
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
