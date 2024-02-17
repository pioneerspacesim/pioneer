-- Copyright © 2008-2023 Pioneer Developers. See AUTHORS.txt for details
-- Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

-- Entries for the FlightLog

local Game = require 'Game'
local Format = require 'Format'

local utils = require 'utils'
local Serializer = require 'Serializer'

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
---@field protected sort_date number 			Seconds since the epoch in game time, used to sort when loading old style
---@field protected entry string 				User entered text associated with the entry
---@field protected always_custom boolean?	 	Is this always treated as a custom entry (so not auto deleted)
FlightLogEntry.Base = utils.class("FlightLogEntry")

---@return string Description of this type
function FlightLogEntry.Base:GetType() end

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

---@return boolean true if this can be removed from the flightlog method
function FlightLogEntry.Base:CanBeRemoved()
	return false
end


---@return boolean True if this log entry should be considered custom and so not auto deleted
function FlightLogEntry.Base:IsCustom()
	return self.always_custom or self.entry and self.entry ~= ""
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

---@return string Description of this type
function FlightLogEntry.System:GetType() 
	return "System"
end


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

function FlightLogEntry.System:Serialize()
	return { systemp = self.systemp, arrtime = self.arrtime, deptime = self.deptime, entry = self.entry }
end

function FlightLogEntry.System.Unserialize( data )
	return FlightLogEntry.System.New(data.systemp, data.arrtime, data.deptime, data.entry )
end

Serializer:RegisterClass("FlightLogEntry.System", FlightLogEntry.System)

---@return string The name for this log entry type
function FlightLogEntry.System:GetLocalizedName()
	return l.LOG_SYSTEM;
end

local function asFaction(path)
	if path:IsSectorPath() then return l.UNKNOWN_FACTION end
	return path:GetStarSystem().faction.name
end

local function asStation(path)
	if not path:IsBodyPath() then return l.NO_AVAILABLE_DATA end
	local system = path:GetStarSystem()
	local systembody = system:GetBodyByPath(path)
	local station_type = "FLIGHTLOG_" .. systembody.type
	return string.interp(l[station_type], {
		primary_info = systembody.name,
		secondary_info = systembody.parent.name
	})
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
	table.insert(o, { l.ALLEGIANCE, asFaction(self.systemp) })

	return o
end

---@class FlightLogEntry.Custom : FlightLogEntry.Base
---@field systemp 	SystemPath	The system the player is in when the log was written
---@field time		number		The game time the log was made, relative to the epoch
---@field money		integer		The amount of money the player has
---@field location	string[]	A number of string elements that can be compsed to create a localized description of the location.  See composeLocationString
FlightLogEntry.Custom = utils.class("FlightLogEntry.Custom", FlightLogEntry.Base)

---@return string Description of this type
function FlightLogEntry.Custom:GetType() 
	return "Custom"
end

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

function FlightLogEntry.Custom:Serialize()
	return { systemp = self.systemp, time = self.time, money = self.money, location = self.location, entry = self.entry }
end

function FlightLogEntry.Custom.Unserialize( data )
	return FlightLogEntry.Custom.New( data.systemp, data.time, data.money, data.location, data.entry )
end

Serializer:RegisterClass("FlightLogEntry.Custom", FlightLogEntry.Custom)

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
		{ l.ALLEGIANCE, asFaction(self.systemp) },
		{ l.CASH, Format.Money(self.money) }
	}
end

---@return boolean true if this has a Delete() method
function FlightLogEntry.Custom:CanBeRemoved()
	return true
end

---@class FlightLogEntry.Station : FlightLogEntry.Base
---@field systemp 	SystemPath	The system the player is in when the log was written
---@field time		deptime		The game time the log was made, on departure from teh system, relative to the epoch
---@field money		integer		The amount of money the player has
FlightLogEntry.Station = utils.class("FlightLogEntry.Station", FlightLogEntry.Base)

---@return string Description of this type
function FlightLogEntry.Station:GetType() 
	return "Station"
end

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

function FlightLogEntry.Station:Serialize()
	return { systemp = self.systemp, deptime = self.deptime, money = self.money, entry = self.entry }
end

function FlightLogEntry.Station.Unserialize( data )
	return FlightLogEntry.Station.New( data.systemp, data.deptime, data.money, data.entry )
end

Serializer:RegisterClass("FlightLogEntry.Station", FlightLogEntry.Station)


---@return string The name for this log entry type
function FlightLogEntry.Station:GetLocalizedName()
	return l.LOG_STATION;
end

---@param earliest_first boolean set to true if your sort order is to show the earlist first dates
---@return table<string, string>[] An array of key value pairs, the key being localized and the value being formatted appropriately.
function FlightLogEntry.Station:GetDataPairs( earliest_first )

	return {
		{ l.DATE, self.formatDate(self.deptime) },
		{ l.STATION, asStation(self.systemp) },
		{ l.IN_SYSTEM, ui.Format.SystemPath(self.systemp) },
		{ l.ALLEGIANCE, asFaction(self.systemp) },
		{ l.CASH, Format.Money(self.money) },
	}
end

return FlightLogEntry