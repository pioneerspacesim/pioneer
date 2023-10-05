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
---@type integer
local MaxTotalDefaultElements = 3000
-- how many default (so not custom) elements do we have
---@type integer
local TotalDefaultElements = 0

-- private data - the log itself
---@type LogEntry[]
local FlightLogData = {}

---@class LogEntry
---A generic log entry:
---@field protected sort_date number Seconds since the epoch in game time, used to sort when loading old style
---@field protected entry string User entered text associated with the entry
---@field protected always_custom boolean|nil Is this always treated as a custom entry (so not auto deleted)
LogEntry = utils.class("FlightLog.LogEntry.Base")

---@return string Description of this type
function LogEntry:GetType()
	local full_type = self.Class().meta.class
	return string.sub( full_type, #"FlightLog.LogEntry."+1, #full_type )
end

---@return boolean true if this is considered to have an entry
function LogEntry:CanHaveEntry()
	return true
end

---@return boolean true if this has an entry
function LogEntry:HasEntry()
	return self.entry and #self.entry > 0
end

---@return string The user provided entry or an empty string if there isn't one.
function LogEntry:GetEntry()
	if not self.entry then return "" end
	return self.entry
end

---@return boolean true if this has a Delete() method
function LogEntry:SupportsDelete()
	return false
end


---@return boolean True if this log entry should be considered custom and so not auto deleted
function LogEntry:IsCustom()
	if self.always_custom then return true end

	if not self.entry then return false end
	if self.entry == "" then return false end
	return true
end

-- The serialization table is an array
-- the elements of the array are a key-value pair
-- the key is the serialization index (so the type this log entry is)
-- the value is an array of data, to construct the table from.
---@param table table<integer, any[]>
---@return nil
function LogEntry:AddToSerializationTable( table )
	local v = {}
	v[self.GetSerializationIndex()] = self:GetSerializationElements()
	table[#table+1] = v
end

---@return integer A unique integer for this specific type
function LogEntry.GetSerializationIndex()
	return -1
end

---@return any[] An array of the elements as they will be serialized
function LogEntry:GetSerializationElements()
	return {}
end


---@return string The name for this log entry type
function LogEntry:GetLocalizedName()
	return "Error"
end

---@param earliest_first boolean set to true if your sort order is to show the earlist first dates
---@return table<string, string>[] An array of key value pairs, the key being localized and the value being formatted appropriately.
function LogEntry:GetDataPairs( earliest_first )
	return { { "ERROR", "This should never be seen" } }
end

---@param entry string A user provided description of the event.
---If non nil/empty this will cause the entry to be considered custom and not automatically deleted
---@return nil
function LogEntry:UpdateEntry( entry )

	if self:IsCustom() then
		TotalDefaultElements = TotalDefaultElements-1
	end

	if entry and #entry == 0 then entry = nil end
	self.entry = entry

	if self:IsCustom() then
		TotalDefaultElements = TotalDefaultElements+1
	end

end

---@param sort_date number The date to sort by (from epoch)
---@param entry string|nil The user entered custom test for this entry
---@param always_custom boolean Is this always treated as a custom entry
function LogEntry:Constructor( sort_date, entry, always_custom )
	-- a date that can be used to sort entries on TODO: remove this
	self.sort_date = sort_date
	-- the entry text associated with this log entry
	if entry and #entry == 0 then entry = nil end
	self.entry = entry
	self.always_custom = always_custom
	if self:IsCustom() then
		TotalDefaultElements = TotalDefaultElements+1
	end
end


--- convenience helper function
--- Sometimes date is empty, e.g. departure date prior to departure
--- TODO: maybe not return this at all then!
---
---@param date number The date since the epoch
---
---@return string  The date formatted
function LogEntry.formatDate(date)
	return date and Format.Date(date) or nil
end

--- Based on flight state, compose a reasonable string for location
--- TODO: consider a class to represent, construct, store and format this
---@param location string[] Array of string info, the first one is the  
---@return string The formatted composite location.
function LogEntry.composeLocationString(location)
	return string.interp(l["FLIGHTLOG_"..location[1]],
		{ primary_info = location[2],
			secondary_info = location[3] or "",
			tertiary_info = location[4] or "",})
end

---@class CurrentStatusLogEngtry : LogEntry
--- Does not have any members, it grabs the current status live whenever requested
CurrentStatusLogEntry = utils.class("FilghtLog.LogEntry.CurrentStatus", LogEntry )

---@return boolean true if this is considered to have an entry
function CurrentStatusLogEntry:CanHaveEntry()
	return false
end

function CurrentStatusLogEntry:Constructor()
	LogEntry.Constructor( self, Game.time, nil, true )
end

---@return string The name for this log entry type
function CurrentStatusLogEntry:GetLocalizedName()
	return l.PERSONAL_INFORMATION;
end

---@param earliest_first boolean set to true if your sort order is to show the earlist first dates
---@return table<string, string>[] An array of key value pairs, the key being localized and the value being formatted appropriately.
function CurrentStatusLogEntry:GetDataPairs( earliest_first )
    local player = Character.persistent.player

	return {
		{ l.NAME_PERSON, player.name },
		-- TODO: localize
		{ "Title", player.title },
		{ l.RATING, l[player:GetCombatRating()] },
		{ l.KILLS,  string.format('%d',player.killcount) }
	}
end

---@class SystemLogEntry : LogEntry
---@field systemp SystemPath 	The system in question
---@field arrtime number|nil	The time of arrival in the system, nil if this is an exit log
---@field depime number|nil	The time of leaving the system, nil if this is an entry log
SystemLogEntry = utils.class("FlightLog.LogEntry.System", LogEntry)


---@param systemp SystemPath 	The system in question
---@param arrtime number|nil	The time of arrival in the system, nil if this is an exit log
---@param depime number|nil	The time of leaving the system, nil if this is an entry log
---@param entry string The user entered custom test for this entry
function SystemLogEntry:Constructor( systemp, arrtime, deptime, entry )

	local sort_date
	if nil == arrtime then
		sort_date = deptime
	else
		sort_date = arrtime
	end

	LogEntry.Constructor( self, sort_date, entry )

	self.systemp = systemp
	self.arrtime = arrtime
	self.deptime = deptime

end

---@return integer A unique integer for this specific type
function SystemLogEntry.GetSerializationIndex()
	return 0
end

---@return any[] An array of the elements as they will be serialized
function SystemLogEntry:GetSerializationElements()
	return { self.systemp, self.arrtime, self.deptime, self.entry }
end

--- A static function to create an entry from the elements that have been serialized
--- For the latest version will be the opposite of GetSerializationElements()
---@param elem 		any[] 				An array of elements used to construct
---@param version 	integer				The version to read
---@return 			SystemLogEntry		The newly created entry
function SystemLogEntry.CreateFromSerializationElements( elem, version )
	return SystemLogEntry.New( elem[1], elem[2], elem[3], elem[4] )
end

---@return string The name for this log entry type
function SystemLogEntry:GetLocalizedName()
	return l.LOG_SYSTEM;
end

---@param earliest_first boolean set to true if your sort order is to show the earlist first dates
---@return table<string, string>[] An array of key value pairs, the key being localized and the value being formatted appropriately.
function SystemLogEntry:GetDataPairs( earliest_first )
	local o = {} ---@type table<string, string>[]

	if ( earliest_first ) then
		if self.arrtime then
			o[#o+1] = { l.ARRIVAL_DATE, self.formatDate(self.arrtime) }
		end
		if self.deptime then
			o[#o+1] = { l.DEPARTURE_DATE, self.formatDate(self.deptime) }
		end
	else
		if self.deptime then
			o[#o+1] = { l.DEPARTURE_DATE, self.formatDate(self.deptime) }
		end		
		if self.arrtime then
			o[#o+1] = { l.ARRIVAL_DATE, self.formatDate(self.arrtime) }
		end
	end
	o[#o+1] = { l.IN_SYSTEM, ui.Format.SystemPath(self.systemp) }
	o[#o+1] = { l.ALLEGIANCE, self.systemp:GetStarSystem().faction.name }

	return o
end

---@class CustomLogEntry : LogEntry
---@field systemp 	SystemPath	The system the player is in when the log was written
---@field time		number		The game time the log was made, relative to the epoch
---@field money		integer		The amount of money the player has
---@field location	string[]	A number of string elements that can be compsed to create a localized description of the location.  See composeLocationString
CustomLogEntry = utils.class("FlightLog.LogEntry.Custom", LogEntry)


---@param systemp SystemPath 	The system in question
---@param time		number		The game time the log was made, relative to the epoch
---@param money		integer		The amount of money the player has
---@param location	string[]	A number of string elements that can be compsed to create a localized description of the location.  See composeLocationString
---@param entry string The user entered custom test for this entry
function CustomLogEntry:Constructor( systemp, time, money, location, entry )
	LogEntry.Constructor( self, time, entry, true )

	self.systemp = systemp
	self.time = time
	self.money = money
	self.location = location
end

---@return integer A unique integer for this specific type
function CustomLogEntry.GetSerializationIndex()
	return 1
end

---@return any[] An array of the elements as they will be serialized
function CustomLogEntry:GetSerializationElements()
	return { self.systemp, self.time, self.money, self.location, self.entry }
end

--- A static function to create an entry from the elements that have been serialized
--- For the latest version will be the opposite of GetSerializationElements()
---@param elem 		any[] 	An array of elements used to construct
---@param version 	integer	The version to read
function CustomLogEntry.CreateFromSerializationElements( elem, version )
	return CustomLogEntry.New( elem[1], elem[2], elem[3], elem[4], elem[5] )
end

---@return string The name for this log entry type
function CustomLogEntry:GetLocalizedName()
	return l.LOG_CUSTOM;
end

---@param earliest_first boolean set to true if your sort order is to show the earlist first dates
---@return table<string, string>[] An array of key value pairs, the key being localized and the value being formatted appropriately.
function CustomLogEntry:GetDataPairs( earliest_first )
	return {
		{ l.DATE, self.formatDate(self.time) },
		{ l.LOCATION, self.composeLocationString(self.location) },
		{ l.IN_SYSTEM, ui.Format.SystemPath(self.systemp) },
		{ l.ALLEGIANCE, self.systemp:GetStarSystem().faction.name },
		{ l.CASH, Format.Money(self.money) }
	}
end

---@return boolean true if this has a Delete() method
function CustomLogEntry:SupportsDelete()
	return true
end

---Delete this entry
---@return nil
function CustomLogEntry:Delete()
	TotalDefaultElements = TotalDefaultElements - 1
	utils.remove_elem( FlightLogData, self )
end

---@class StationLogEntry : LogEntry
---@field systemp 	SystemPath	The system the player is in when the log was written
---@field time		deptime		The game time the log was made, on departure from teh system, relative to the epoch
---@field money		integer		The amount of money the player has
StationLogEntry = utils.class("FlightLog.LogEntry.Station", LogEntry)

---@param systemp 	SystemPath	The system the player is in when the log was written
---@param time		deptime		The game time the log was made, on departure from teh system, relative to the epoch
---@param money		integer		The amount of money the player has
---@param entry string The user entered custom test for this entry
function StationLogEntry:Constructor( systemp, deptime, money, entry )
	LogEntry.Constructor( self, deptime, entry )

	self.systemp = systemp
	self.deptime = deptime
	self.money = money
end

---@return integer A unique integer for this specific type
function StationLogEntry.GetSerializationIndex()
	return 2
end

---@return any[] An array of the elements as they will be serialized
function StationLogEntry:GetSerializationElements()
	return { self.systemp, self.deptime, self.money, self.entry }
end

--- A static function to create an entry from the elements that have been serialized
--- For the latest version will be the opposite of GetSerializationElements()
---@param elem 		any[] 	An array of elements used to construct
---@param version 	integer	The version to read
function StationLogEntry.CreateFromSerializationElements( elem, version )
	return StationLogEntry.New( elem[1], elem[2], elem[3], elem[4] )
end

---@return string The name for this log entry type
function StationLogEntry:GetLocalizedName()
	return l.LOG_STATION;
end

---@param earliest_first boolean set to true if your sort order is to show the earlist first dates
---@return table<string, string>[] An array of key value pairs, the key being localized and the value being formatted appropriately.
function StationLogEntry:GetDataPairs( earliest_first )

	local station_type = "FLIGHTLOG_" .. self.systemp:GetSystemBody().type

	return {
		{ l.DATE, self.formatDate(self.deptime) },
		{ l.STATION, string.interp(l[station_type],
		{	primary_info = self.systemp:GetSystemBody().name,
			secondary_info = self.systemp:GetSystemBody().parent.name }) },
		{ l.IN_SYSTEM, ui.Format.SystemPath(self.systemp) },
		{ l.ALLEGIANCE, self.systemp:GetStarSystem().faction.name },
		{ l.CASH, Format.Money(self.money) },
	}
end


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

		table.insert(FlightLogData,1, CustomLogEntry.New( path, Game.time, Game.player:GetMoney(), location, text ) )
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
function FlightLog:GetLogEntries(types, maximum, earliest_first)

	-- TODO: actually just store a list of all of them as they are at startup
	local type_set = utils.set.New(types)

	-- note regardless of sort order, current status always comes first.
	local currentStatus = nil
	if nil == types or type_set:contains( "CurrentStatus" ) then
		currentStatus = CurrentStatusLogEntry.New()
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
	local system_idx = SystemLogEntry.GetSerializationIndex(); ---@type integer

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
	if TotalDefaultElements > MaxTotalDefaultElements then
		CollapseSystemEvents()
		while TotalDefaultElements > MaxTotalDefaultElements do
			for i = #FlightLogData, 1, -1 do
				local v = FlightLogData[i]
				if not v:IsCustom() then
					table.remove( FlightLogData, i )
					TotalDefaultElements = TotalDefaultElements-1
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
	
	table.insert( FlightLogData, 1, SystemLogEntry.New( Game.system.path, nil, Game.time, nil ) );
	ConsiderCollapseSystemEventPair( 1 )
	TrimLogSize()
end

-- onEnterSystem
local AddSystemArrivalToLog = function (ship)
	if not ship:IsPlayer() then return end

	table.insert( FlightLogData, 1, SystemLogEntry.New( Game.system.path, Game.time, nil, nil ) );
	TrimLogSize()
end

-- onShipDocked
local AddStationToLog = function (ship, station)
	if not ship:IsPlayer() then return end

	table.insert( FlightLogData, 1, StationLogEntry.New( station.path, Game.time, Game.player:GetMoney(), nil ) );
	TrimLogSize()
end

-- LOADING AND SAVING

local loaded_data

local onGameStart = function ()

	if loaded_data and loaded_data.Version == 1 then

		for _, v in pairs( loaded_data.System ) do		
			local entryLog = SystemLogEntry.CreateFromSerializationElements( { v[1], v[2], nil, v[4] }, 1 )
			local exitLog = SystemLogEntry.CreateFromSerializationElements( { v[1], nil, v[3], v[4] }, 1 )

			if (exitLog.deptime ~= nil) then
				FlightLogData[#FlightLogData+1]	= exitLog
			end
			if (entryLog.arrtime ~= nil) then
				FlightLogData[#FlightLogData+1]	= entryLog
			end
		end

		for _, v in pairs( loaded_data.Station ) do
			FlightLogData[#FlightLogData+1] = StationLogEntry.CreateFromSerializationElements( v, 1 ) 		
		end
		
		for _, v in pairs( loaded_data.Custom ) do
			FlightLogData[#FlightLogData+1] = CustomLogEntry.CreateFromSerializationElements( v, 1 )		
		end

		local function sortf( a, b )
			return a.sort_date > b.sort_date
		end
	
		table.sort( FlightLogData, sortf )

		CollapseSystemEvents()

	elseif loaded_data and loaded_data.Version > 1 then

		local loader_funcs = {}
		loader_funcs[SystemLogEntry.GetSerializationIndex()] = SystemLogEntry.CreateFromSerializationElements
		loader_funcs[StationLogEntry.GetSerializationIndex()] = StationLogEntry.CreateFromSerializationElements
		loader_funcs[CustomLogEntry.GetSerializationIndex()] = CustomLogEntry.CreateFromSerializationElements
	
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
	TotalDefaultElements = 0
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
