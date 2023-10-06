-- Copyright © 2008-2023 Pioneer Developers. See AUTHORS.txt for details
-- Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

-- Entries for the FlightLog

local Game = require 'Game'
local Format = require 'Format'

local utils = require 'utils'

local Character = require 'Character'

-- required for formatting / localisation
local ui = require 'pigui'
local Lang = require 'Lang'
local l = Lang.GetResource("ui-core")
-- end of formating / localisation stuff

FlightLogEntry = {}

-- how many default (so not custom) elements do we have
---@type integer
FlightLogEntry.TotalDefaultElements = 0


---@class FlightLogEntry.Base
---A generic log entry:
---@field protected sort_date number Seconds since the epoch in game time, used to sort when loading old style
---@field protected entry string User entered text associated with the entry
---@field protected always_custom boolean|nil Is this always treated as a custom entry (so not auto deleted)
FlightLogEntry.Base = utils.class("FlightLogEntry")

---@return string Description of this type
function FlightLogEntry.Base:GetType()
	local full_type = self.Class().meta.class
	return string.sub( full_type, #"FlightLogEntry."+1, #full_type )
end

---@return boolean true if this is considered to have an entry
function FlightLogEntry.Base:CanHaveEntry()
	return true
end

---@return boolean true if this has an entry
function FlightLogEntry.Base:HasEntry()
	return self.entry and #self.entry > 0
end

---@return string The user provided entry or an empty string if there isn't one.
function FlightLogEntry.Base:GetEntry()
	if not self.entry then return "" end
	return self.entry
end

---@return boolean true if this has a Delete() method
function FlightLogEntry.Base:SupportsDelete()
	return false
end


---@return boolean True if this log entry should be considered custom and so not auto deleted
function FlightLogEntry.Base:IsCustom()
	if self.always_custom then return true end

	if not self.entry then return false end
	if self.entry == "" then return false end
	return true
end

-- The serialization table is an array
-- the elements of the array are a key-value pair
-- the key is the serialization index (so the type this log entry is)
-- the value is an array of data, to construct the table from.
---@param out table<integer, any[]>
---@return nil
function FlightLogEntry.Base:AddToSerializationTable( out )
	local v = {}
	v[self.GetSerializationIndex()] = self:GetSerializationElements()
	table.insert(out, v)
end

---@return integer A unique integer for this specific type
function FlightLogEntry.Base.GetSerializationIndex()
	return -1
end

---@return any[] An array of the elements as they will be serialized
function FlightLogEntry.Base:GetSerializationElements()
	return {}
end


---@return string The name for this log entry type
function FlightLogEntry.Base:GetLocalizedName()
	return "Error"
end

---@param earliest_first boolean set to true if your sort order is to show the earlist first dates
---@return table<string, string>[] An array of key value pairs, the key being localized and the value being formatted appropriately.
function FlightLogEntry.Base:GetDataPairs( earliest_first )
	return { { "ERROR", "This should never be seen" } }
end

---@param entry string A user provided description of the event.
---If non nil/empty this will cause the entry to be considered custom and not automatically deleted
---@return nil
function FlightLogEntry.Base:UpdateEntry( entry )

	if self:IsCustom() then
		FlightLogEntry.TotalDefaultElements = FlightLogEntry.TotalDefaultElements-1
	end

	if entry and #entry == 0 then entry = nil end
	self.entry = entry

	if self:IsCustom() then
		FlightLogEntry.TotalDefaultElements = FlightLogEntry.TotalDefaultElements+1
	end

end

---@param sort_date number The date to sort by (from epoch)
---@param entry string|nil The user entered custom test for this entry
---@param always_custom boolean Is this always treated as a custom entry
function FlightLogEntry.Base:Constructor( sort_date, entry, always_custom )
	-- a date that can be used to sort entries on TODO: remove this
	self.sort_date = sort_date
	-- the entry text associated with this log entry
	if entry and #entry == 0 then entry = nil end
	self.entry = entry
	self.always_custom = always_custom
	if self:IsCustom() then
		FlightLogEntry.TotalDefaultElements = FlightLogEntry.TotalDefaultElements+1
	end
end


--- convenience helper function
--- Sometimes date is empty, e.g. departure date prior to departure
--- TODO: maybe not return this at all then!
---
---@param date number The date since the epoch
---
---@return string  The date formatted
function FlightLogEntry.Base.formatDate(date)
	return date and Format.Date(date) or nil
end

--- Based on flight state, compose a reasonable string for location
--- TODO: consider a class to represent, construct, store and format this
---@param location string[] Array of string info, the first one is the  
---@return string The formatted composite location.
function FlightLogEntry.Base.composeLocationString(location)
	return string.interp(l["FLIGHTLOG_"..location[1]],
		{ primary_info = location[2],
			secondary_info = location[3] or "",
			tertiary_info = location[4] or "",})
end

---@class CurrentStatusLogEngtry : FlightLogEntry.Base
--- Does not have any members, it grabs the current status live whenever requested
FlightLogEntry.CurrentStatus = utils.class("FlightLogEntry.CurrentStatus", FlightLogEntry.Base )

---@return boolean true if this is considered to have an entry
function FlightLogEntry.CurrentStatus:CanHaveEntry()
	return false
end

function FlightLogEntry.CurrentStatus:Constructor()
	FlightLogEntry.Base.Constructor( self, Game.time, nil, true )
end

---@return string The name for this log entry type
function FlightLogEntry.CurrentStatus:GetLocalizedName()
	return l.PERSONAL_INFORMATION;
end

---@param earliest_first boolean set to true if your sort order is to show the earlist first dates
---@return table<string, string>[] An array of key value pairs, the key being localized and the value being formatted appropriately.
function FlightLogEntry.CurrentStatus:GetDataPairs( earliest_first )
    local player = Character.persistent.player

	return {
		{ l.NAME_PERSON, player.name },
		-- TODO: localize
		{ "Title", player.title },
		{ l.RATING, l[player:GetCombatRating()] },
		{ l.KILLS,  string.format('%d',player.killcount) }
	}
end

---@class FlightLogEntry.System : FlightLogEntry.Base
---@field systemp SystemPath 	The system in question
---@field arrtime number|nil	The time of arrival in the system, nil if this is an exit log
---@field depime number|nil	The time of leaving the system, nil if this is an entry log
FlightLogEntry.System = utils.class("FlightLogEntry.System", FlightLogEntry.Base)


---@param systemp SystemPath 	The system in question
---@param arrtime number|nil	The time of arrival in the system, nil if this is an exit log
---@param depime number|nil	The time of leaving the system, nil if this is an entry log
---@param entry string The user entered custom test for this entry
function FlightLogEntry.System:Constructor( systemp, arrtime, deptime, entry )

	local sort_date
	if nil == arrtime then
		sort_date = deptime
	else
		sort_date = arrtime
	end

	FlightLogEntry.Base.Constructor( self, sort_date, entry )

	self.systemp = systemp
	self.arrtime = arrtime
	self.deptime = deptime

end

---@return integer A unique integer for this specific type
function FlightLogEntry.System.GetSerializationIndex()
	return 0
end

---@return any[] An array of the elements as they will be serialized
function FlightLogEntry.System:GetSerializationElements()
	return { self.systemp, self.arrtime, self.deptime, self.entry }
end

--- A static function to create an entry from the elements that have been serialized
--- For the latest version will be the opposite of GetSerializationElements()
---@param elem 		any[] 				An array of elements used to construct
---@param version 	integer				The version to read
---@return 			FlightLogEntry.System		The newly created entry
function FlightLogEntry.System.CreateFromSerializationElements( elem, version )
	return FlightLogEntry.System.New( elem[1], elem[2], elem[3], elem[4] )
end

---@return string The name for this log entry type
function FlightLogEntry.System:GetLocalizedName()
	return l.LOG_SYSTEM;
end

---@param earliest_first boolean set to true if your sort order is to show the earlist first dates
---@return table<string, string>[] An array of key value pairs, the key being localized and the value being formatted appropriately.
function FlightLogEntry.System:GetDataPairs( earliest_first )
	local o = {} ---@type table<string, string>[]

	if ( earliest_first ) then
		if self.arrtime then
			table.insert(o, { l.ARRIVAL_DATE, self.formatDate(self.arrtime) })
		end
		if self.deptime then
			table.insert(o, { l.DEPARTURE_DATE, self.formatDate(self.deptime) })
		end
	else
		if self.deptime then
			table.insert(o, { l.DEPARTURE_DATE, self.formatDate(self.deptime) })
		end		
		if self.arrtime then
			table.insert(o, { l.ARRIVAL_DATE, self.formatDate(self.arrtime) })
		end
	end
	table.insert(o, { l.IN_SYSTEM, ui.Format.SystemPath(self.systemp) })
	table.insert(o, { l.ALLEGIANCE, self.systemp:GetStarSystem().faction.name })

	return o
end

---@class FlightLogEntry.Custom : FlightLogEntry.Base
---@field systemp 	SystemPath	The system the player is in when the log was written
---@field time		number		The game time the log was made, relative to the epoch
---@field money		integer		The amount of money the player has
---@field location	string[]	A number of string elements that can be compsed to create a localized description of the location.  See composeLocationString
FlightLogEntry.Custom = utils.class("FlightLogEntry.Custom", FlightLogEntry.Base)


---@param systemp SystemPath 	The system in question
---@param time		number		The game time the log was made, relative to the epoch
---@param money		integer		The amount of money the player has
---@param location	string[]	A number of string elements that can be compsed to create a localized description of the location.  See composeLocationString
---@param entry string The user entered custom test for this entry
function FlightLogEntry.Custom:Constructor( systemp, time, money, location, entry )
	FlightLogEntry.Base.Constructor( self, time, entry, true )

	self.systemp = systemp
	self.time = time
	self.money = money
	self.location = location
end

---@return integer A unique integer for this specific type
function FlightLogEntry.Custom.GetSerializationIndex()
	return 1
end

---@return any[] An array of the elements as they will be serialized
function FlightLogEntry.Custom:GetSerializationElements()
	return { self.systemp, self.time, self.money, self.location, self.entry }
end

--- A static function to create an entry from the elements that have been serialized
--- For the latest version will be the opposite of GetSerializationElements()
---@param elem 		any[] 	An array of elements used to construct
---@param version 	integer	The version to read
function FlightLogEntry.Custom.CreateFromSerializationElements( elem, version )
	return FlightLogEntry.Custom.New( elem[1], elem[2], elem[3], elem[4], elem[5] )
end

---@return string The name for this log entry type
function FlightLogEntry.Custom:GetLocalizedName()
	return l.LOG_CUSTOM;
end

---@param earliest_first boolean set to true if your sort order is to show the earlist first dates
---@return table<string, string>[] An array of key value pairs, the key being localized and the value being formatted appropriately.
function FlightLogEntry.Custom:GetDataPairs( earliest_first )
	return {
		{ l.DATE, self.formatDate(self.time) },
		{ l.LOCATION, self.composeLocationString(self.location) },
		{ l.IN_SYSTEM, ui.Format.SystemPath(self.systemp) },
		{ l.ALLEGIANCE, self.systemp:GetStarSystem().faction.name },
		{ l.CASH, Format.Money(self.money) }
	}
end

---@return boolean true if this has a Delete() method
function FlightLogEntry.Custom:SupportsDelete()
	return true
end

---Delete this entry
---@return nil
function FlightLogEntry.Custom:Delete()
	FlightLogEntry.TotalDefaultElements = FlightLogEntry.TotalDefaultElements - 1
	utils.remove_elem( FlightLogData, self )
end

---@class FlightLogEntry.Station : FlightLogEntry.Base
---@field systemp 	SystemPath	The system the player is in when the log was written
---@field time		deptime		The game time the log was made, on departure from teh system, relative to the epoch
---@field money		integer		The amount of money the player has
FlightLogEntry.Station = utils.class("FlightLogEntry.Station", FlightLogEntry.Base)

---@param systemp 	SystemPath	The system the player is in when the log was written
---@param time		deptime		The game time the log was made, on departure from teh system, relative to the epoch
---@param money		integer		The amount of money the player has
---@param entry string The user entered custom test for this entry
function FlightLogEntry.Station:Constructor( systemp, deptime, money, entry )
	FlightLogEntry.Base.Constructor( self, deptime, entry )

	self.systemp = systemp
	self.deptime = deptime
	self.money = money
end

---@return integer A unique integer for this specific type
function FlightLogEntry.Station.GetSerializationIndex()
	return 2
end

---@return any[] An array of the elements as they will be serialized
function FlightLogEntry.Station:GetSerializationElements()
	return { self.systemp, self.deptime, self.money, self.entry }
end

--- A static function to create an entry from the elements that have been serialized
--- For the latest version will be the opposite of GetSerializationElements()
---@param elem 		any[] 	An array of elements used to construct
---@param version 	integer	The version to read
function FlightLogEntry.Station.CreateFromSerializationElements( elem, version )
	return FlightLogEntry.Station.New( elem[1], elem[2], elem[3], elem[4] )
end

---@return string The name for this log entry type
function FlightLogEntry.Station:GetLocalizedName()
	return l.LOG_STATION;
end

---@param earliest_first boolean set to true if your sort order is to show the earlist first dates
---@return table<string, string>[] An array of key value pairs, the key being localized and the value being formatted appropriately.
function FlightLogEntry.Station:GetDataPairs( earliest_first )

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

return FlightLogEntry