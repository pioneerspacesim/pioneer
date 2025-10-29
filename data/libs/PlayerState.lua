-- Copyright © 2008-2025 Pioneer Developers. See AUTHORS.txt for details
-- Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

local Event      = require 'Event'
local Game       = require 'Game'
local Serializer = require 'Serializer'
local utils      = require 'utils'

--
-- Interface: Event
--
-- Event: onBookmarkChanged(id)
-- Triggered when the player's bookmarks are modified
--

--
-- Interface: PlayerState
--
-- PlayerState encodes most or all abstract state about the Player as a logical
-- entity. This should represent global state which does not change when the
-- player changes ships, etc.
--
-- For concrete data associated with a specific ship or object, prefer to store
-- the information on the player's current Ship instead.
--

---@alias BookmarkID string

---@class SystemBookmark : table
---@field path SystemPath
---@field route_to SystemPath?
---@field commodity string?
---@field note string?

local PlayerState = {}

-- Store player bookmark state:
---@type table<BookmarkID, SystemBookmark>
local bookmarks = {}
local bookmarkIdx = 0

-- Store player financial status:
local finances = {
	cash = 0
}

-- container for crimes committed
---@class CrimeRecord
---@field clone fun(): CrimeRecord
local CrimeRecord = utils.proto("CrimeRecord")

local function initializeCrimetype(record)
    record.crimetype = setmetatable(record.crimetype or {}, { __index = function(t, crime)
        local crimetype = { count = 0 }
        rawset(t, crime, crimetype)
        return crimetype
    end})
end

function CrimeRecord:__clone()
    initializeCrimetype(self)
    self.fine = 0
end

-- Append another crime record's list of crimes. Does not append the fine.
function CrimeRecord:append(other)
	for crime, v in pairs(other.crimetype) do
		local record = self.crimetype[crime]
		record.count = record.count + v.count
	end
end

-- Automagically create empty crime records on access
local automagic_record = {
	__index = function(t, faction)
		print('__index called on an automagic record!')
		local record = CrimeRecord:__clone()
		rawset(t, faction, record)
		return record
	end
}

-- Store player legal status:
local crime_record = setmetatable({}, automagic_record)
local past_record = setmetatable({}, automagic_record)

--=============================================================================

-- Function: AddBookmark
--
-- Bookmark a specific body or system with optional data
---@param path SystemPath
---@param data table?
---@return BookmarkID id
function PlayerState.AddBookmark(path, data)
	bookmarkIdx = bookmarkIdx + 1
	local id = tostring(bookmarkIdx) .. (":{sectorX}.{sectorY}.{sectorZ}.{systemIndex}:{bodyIndex}" % path)

	local bookmark = { path = path }

	if data then
		table.merge(bookmark, data)
	end

	bookmarks[id] = bookmark
	Event.Queue("onBookmarkChanged", id, bookmark)
	return id
end

-- Function: RemoveBookmark
--
-- Remove an existing bookmark
---@param id BookmarkID
function PlayerState.RemoveBookmark(id)
	local bookmark = bookmarks[id]

	if bookmark then
		bookmarks[id] = nil
		Event.Queue("onBookmarkChanged", id, bookmark)
	end
end

-- Function: ModifyBookmark
--
-- Change the data associated with an existing bookmark
---@param id BookmarkID
---@param bookmark table
function PlayerState.UpdateBookmark(id, bookmark)
	if bookmarks[id] then
		bookmarks[id] = bookmark
	end

	Event.Queue("onBookmarkChanged", id, bookmarks[id])
end

-- Function: GetBookmarks
--
-- Return an iterable table of bookmarks
function PlayerState.GetBookmarks()
	return bookmarks
end

--=============================================================================

-- Function: AddMoney
--
-- Add money to the player's account. Add a negative amount to remove money.
---@param amt number
function PlayerState.AddMoney(amt)
	finances.cash = finances.cash + amt
end

-- Function: SetMoney
--
-- Set the money in the player's account.
---@param amt number
function PlayerState.SetMoney(amt)
	finances.cash = amt
end

-- Function: GetMoney
--
-- Return the amount of money the player currently has.
---@return number money
function PlayerState.GetMoney()
	return finances.cash
end

--=============================================================================

--
-- Function: AddCrime
--
-- Add a crime to the player's criminal record
--
-- > PlayerState.AddCrime(crime, fine, faction)
--
-- Parameters:
--
--   crime - a string constant describing the crime
--
--   fine - an amount to add to the player's fine
--
--   faction - optional argument, defaults to the faction id of the system
--             the player is currently in
--
function PlayerState.AddCrime (crime, fine, faction)
	local forFaction = (faction and faction.id) or Game.system.faction.id
	local record = crime_record[forFaction]


	print('crime_record', getmetatable(crime_record))
	print('record', getmetatable(record))
	print('record.crimetype', getmetatable(record.crimetype))
	print('record.crimetype[crime]', getmetatable(record.crimetype[crime]))

	record.crimetype[crime].count = record.crimetype[crime].count + 1
	record.fine = record.fine + fine
end

--
-- Method: ClearCrimeFine
--
-- Clear the current record of outstanding fines and crimes for player
--
-- > PlayerState.ClearCrimeFine()
-- > PlayerState.ClearCrimeFine(faction)
-- > PlayerState.ClearCrimeFine(faction, forget)
--
-- Parameters:
--
--   faction - optional argument, defaults to the faction id of the system
--             the player is currently in
--
--   forget - optional Boolean argument, defaults to false, resulting in the
--            cleared fines to still be moved to the player's crime record
--            over past offences
--
function PlayerState.ClearCrimeFine (faction, forget)
	local forFaction = (faction and faction.id) or Game.system.faction.id

	if not forget then
		past_record[forFaction]:append(crime_record[forFaction])
	end

	crime_record[forFaction] = nil
end

--
-- Method: ClearCrimeRecordHistory
--
-- Clear the player's crime record history, excluding current unpayed offences
--
-- > PlayerState.ClearCrimeRecordHistory()
-- > PlayerState.ClearCrimeRecordHistory(faction)
--
-- Parameters:
--
--   faction - optional argument, defaults to the faction if of the system
--             the player is currently in
--
function PlayerState.ClearCrimeRecordHistory (faction)
	local forFaction = (faction and faction.id) or Game.system.faction.id
	-- use rawset so we don't create a garbage CrimeRecord
	rawset(past_record, forFaction, nil)
end

--
-- Function: GetCrimeRecord
--
-- Get players past criminal record of and total payed fine for faction
--
-- > criminalRecord, fine = PlayerState.GetCrimeRecord()
-- > criminalRecord, fine = PlayerState.GetCrimeRecord(faction)
--
-- Parameters:
--
--   faction - optional argument, defaults to the faction id of the system
--             the player is in
--
--
-- Return:
--
--   criminalRecord - a table with key being "crime constant" and "count"
--                    for each crime committed
--
--   fine - the total fine of the player in faction
--
function PlayerState.GetCrimeRecord (faction)
	local forFaction = (faction and faction.id) or Game.system.faction.id
	local record = rawget(past_record, forFaction)

	if not record then
		return {}, 0
	elseif Game.player.flightState == "HYPERSPACE" then
		return {}, 0
	else
		return record.crimetype, record.fine
	end
end

-- Function: GetCrimeOutstanding
--
-- Get the player's current outstanding criminal record and total unpaid fine for faction
--
-- > criminalRecord, fine = PlayerState.GetCrimeOutstanding()
-- > criminalRecord, fine = PlayerState.GetCrimeOutstanding(faction)
--
-- Parameters:
--
--   faction - optional argument, defaults to the faction id of the system
--             the player is currently in
--
--
-- Return:
--
--   criminalRecord - a table with key being "crime constant" and "count"
--                    for each crime committed
--
--   fine - the total fine of the player in faction
--
function PlayerState.GetCrimeOutstanding (faction)
	local forFaction = (faction and faction.id) or Game.system.faction.id
	local record = rawget(crime_record, forFaction)

	if not record then
		return {}, 0
	elseif Game.player.flightState == "HYPERSPACE" then
		return {}, 0
	else
		return record.crimetype, record.fine
	end
end

--
-- Function: GetLegalStatus
--
-- > local legal_status = PlayerState.GetLegalStatus(faction)
--
-- Parameters:
--
--   faction - optional argument, defaults to the faction player is in.
--
--   legal_status - one of strings "CLEAN", "OFFENDER", "CRIMINAL", "OUTLAW",
--	                or "FUGITIVE", mapping to translated strings in lang/ui-core
--
function PlayerState.GetLegalStatus (faction)
	local _, fine = PlayerState.GetCrimeOutstanding()

	if fine < 1 then
		return('CLEAN')
	elseif fine < 5500 then
		return('OFFENDER')
	elseif fine < 20000 then
		return('CRIMINAL')
	elseif fine < 100000 then
		return('OUTLAW')
	else
		return('FUGITIVE')
	end
end

--=============================================================================

local function onGameStart()
	local player = Game.player

	-- v91 compatibility
	if player and player:hasprop("cash") and player["cash"] ~= 0 then
		finances.cash = player["cash"]
	end
end

local function onGameEnd()
	bookmarkIdx = 0
	bookmarks = {}
	finances.cash = 0

	crime_record = setmetatable({}, automagic_record)
	past_record = setmetatable({}, automagic_record)
end

---@return table
local function serialize()
	local data = {
		bookmarkIdx = bookmarkIdx,
		bookmarks = bookmarks,
		finances = finances,
		crime_record = crime_record,
		past_record = past_record,
	}

	return data
end

---@param data table
local function unserialize(data)
	-- SAVEBUMP: compatibility with v91 save before PlayerStateDB merge
	if next(data) == nil then return end

	bookmarkIdx = data.bookmarkIdx
	bookmarks = data.bookmarks
	finances = data.finances

	crime_record = setmetatable(data.crime_record, automagic_record)
	past_record = setmetatable(data.past_record, automagic_record)

	for _, record in pairs(crime_record) do
		setmetatable(record, CrimeRecord)
		initializeCrimetype(record)
	end
	for _, record in pairs(past_record) do
		setmetatable(record, CrimeRecord)
		initializeCrimetype(record)
	end
end

Event.Register("onGameStart", onGameStart)
Event.Register("onGameEnd", onGameEnd)

Serializer:Register("PlayerStateDB", serialize, unserialize)
Serializer:RegisterClass("CrimeRecord", CrimeRecord)

-- SAVEBUMP: remove when >v91
if Game.CurrentSaveVersion() == 91 then

	local function unserialize_player(data)
		if data.record then
			print('setting the metatables.')
			crime_record = setmetatable(data.record, automagic_record)
			past_record = setmetatable(data.record_old, automagic_record)
		end
	end

	local function _serialize() return nil end

	Serializer:Register("Player", _serialize, unserialize_player)

end

return PlayerState
