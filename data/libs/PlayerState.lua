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

local bookmarkIdx = 0
---@type table<BookmarkID, SystemBookmark>
local bookmarks = {}

local finances = {
	cash = 0
}

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
end

---@return table
local function serialize()
	local data = {
		bookmarkIdx = bookmarkIdx,
		bookmarks = bookmarks,
		finances = finances
	}

	return data
end

---@param data table
local function unserialize(data)
	bookmarkIdx = data.bookmarkIdx or bookmarkIdx
	bookmarks = data.bookmarks or bookmarks
	finances = data.finances or finances
end

Event.Register("onGameStart", onGameStart)
Event.Register("onGameEnd", onGameEnd)

Serializer:Register("PlayerStateDB", serialize, unserialize)

return PlayerState
