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

local FlightLogEntry = require 'modules.FlightLog.FlightLogEntries'


---@type boolean|nil  If true then we've just started a game so don't record the first docking callback
local skip_first_docking = nil

-- default values (private)
---@type integer
local MaxTotalDefaultElements = 3000

-- private data - the log itself
---@type FlightLogEntry.Base[]
local FlightLogData = {}

local FlightLog
FlightLog = {

--
-- Group: Methods
--
--
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

		table.insert(FlightLogData,1, FlightLogEntry.Custom.New( path, Game.time, Game.player:GetMoney(), location, text ) )
	end,

}



function FlightLog.SkipFirstDocking()
	skip_first_docking = true
end
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
function FlightLog:GetLogEntries(types, maximum, earliest_first)

	-- TODO: actually just store a list of all of them as they are at startup
	local type_set = utils.set.New(types)

	-- note regardless of sort order, current status always comes first.
	local currentStatus = nil
	if nil == types or type_set:contains( "CurrentStatus" ) then
		currentStatus = FlightLogEntry.CurrentStatus.New()
	end

	local counter = 0
	maximum = maximum or #FlightLogData
	return function ()
		if currentStatus then
			local t = currentStatus
			currentStatus = nil
			return t
		end
		while counter < maximum do
			counter = counter + 1

			local v
			if earliest_first then
				v = FlightLogData[(#FlightLogData+1) - counter]				
			else
				v = FlightLogData[counter]
			end			
			-- TODO: Can we map the types to serialization indexes and check these
			-- as they may be faster than the string manipulation comapare stuff.
			if nil == types or type_set:contains( v:GetType() ) then
				return v
			end
		end
		return nil
	end
end

--- If there are two system eventsm back to back, starting at first_index
--- entering and leaving the same system, it will put them together
--- as a single system event
---
---@param first_index integer The index of the first element in the array (so the latest event) to collapse
local function ConsiderCollapseSystemEventPair( first_index )
	-- TODO: make this global (ideally const, but our Lua version doesn't support that)
	local system_idx = FlightLogEntry.System.GetSerializationIndex(); ---@type integer

	local second = FlightLogData[first_index] 
	local first = FlightLogData[first_index+1]

	if ( second:IsCustom() ) then return end
	if ( second.GetSerializationIndex() ~= system_idx ) then return end
	---@cast second SystemLogEntry

	-- is the latest one actually an arrival event, or already collapsed.
	if ( second.arrtime ~= nil ) then return end

--	local first = FlightLogData[first_index+1]
	if ( first:IsCustom() ) then return end
	if ( first.GetSerializationIndex() ~= system_idx ) then return end
	---@cast first SystemLogEntry

	-- is the first one actually a departure event or already collapsed
	if ( first.deptime ~= nil ) then return end

	if ( first.systemp ~= second.systemp ) then return end

	second.arrtime = first.arrtime
	table.remove( FlightLogData, first_index+1 )

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
	if FlightLogEntry.TotalDefaultElements > MaxTotalDefaultElements then
		CollapseSystemEvents()
		while FlightLogEntry.TotalDefaultElements > MaxTotalDefaultElements do
			for i = #FlightLogData, 1, -1 do
				local v = FlightLogData[i]
				if not v:IsCustom() then
					table.remove( FlightLogData, i )
					FlightLogEntry.TotalDefaultElements = FlightLogEntry.TotalDefaultElements-1
				end
			end	
		end
		CollapseSystemEvents()
	end
end


-- LOGGING

-- onLeaveSystem
local AddSystemDepartureToLog = function (ship)
	if not ship:IsPlayer() then return end
	
	table.insert( FlightLogData, 1, FlightLogEntry.System.New( Game.system.path, nil, Game.time, nil ) );
	ConsiderCollapseSystemEventPair( 1 )
	TrimLogSize()
end

-- onEnterSystem
local AddSystemArrivalToLog = function (ship)
	if not ship:IsPlayer() then return end

	table.insert( FlightLogData, 1, FlightLogEntry.System.New( Game.system.path, Game.time, nil, nil ) );
	TrimLogSize()
end

-- onShipDocked
local AddStationToLog = function (ship, station)
	if not ship:IsPlayer() then return end

	-- could check the game time and see if it's the same as the last custom event
	-- and there is nothing else in the list and avoud the phantom 'first docking'
	-- that way too.
	if skip_first_docking then
		skip_first_docking = nil
		return
	end
	table.insert( FlightLogData, 1, FlightLogEntry.Station.New( station.path, Game.time, Game.player:GetMoney(), nil ) );
	TrimLogSize()
end

-- LOADING AND SAVING

local loaded_data

local onGameStart = function ()

	if loaded_data and loaded_data.Version == 1 then

		for _, v in pairs( loaded_data.System ) do		
			local entryLog = FlightLogEntry.System.CreateFromSerializationElements( { v[1], v[2], nil, v[4] }, 1 )
			local exitLog = FlightLogEntry.System.CreateFromSerializationElements( { v[1], nil, v[3], v[4] }, 1 )

			if (exitLog.deptime ~= nil) then
				FlightLogData[#FlightLogData+1]	= exitLog
			end
			if (entryLog.arrtime ~= nil) then
				FlightLogData[#FlightLogData+1]	= entryLog
			end
		end

		for _, v in pairs( loaded_data.Station ) do
			FlightLogData[#FlightLogData+1] = FlightLogEntry.Station.CreateFromSerializationElements( v, 1 ) 		
		end
		
		for _, v in pairs( loaded_data.Custom ) do
			FlightLogData[#FlightLogData+1] = FlightLogEntry.Custom.CreateFromSerializationElements( v, 1 )		
		end

		local function sortf( a, b )
			return a.sort_date > b.sort_date
		end
	
		table.sort( FlightLogData, sortf )

		CollapseSystemEvents()

	elseif loaded_data and loaded_data.Version > 1 then

		local loader_funcs = {}
		loader_funcs[FlightLogEntry.System.GetSerializationIndex()] = FlightLogEntry.System.CreateFromSerializationElements
		loader_funcs[FlightLogEntry.Station.GetSerializationIndex()] = FlightLogEntry.Station.CreateFromSerializationElements
		loader_funcs[FlightLogEntry.Custom.GetSerializationIndex()] = FlightLogEntry.Custom.CreateFromSerializationElements
	
		for _, p in pairs( loaded_data.Data ) do
			for type, v in pairs(p) do
				local lf = loader_funcs[type]
				local val = lf(v, loaded_data.Version);
				FlightLogData[#FlightLogData+1] = val
			end
		end
	end

	loaded_data = nil
end

local onGameEnd = function ()
	FlightLogData = {}
	FlightLogEntry.TotalDefaultElements = 0
end

local serialize = function ()

	local source = FlightLogData
	local SaveData = {}

	for _, v in pairs( source ) do
		v:AddToSerializationTable( SaveData )
	end

	return { 
		Data = SaveData,
		Version = 2 -- version for backwards compatibility
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
