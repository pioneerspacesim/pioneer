-- Copyright © 2008-2024 Pioneer Developers. See AUTHORS.txt for details
-- Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

--
-- Interface: Player
--
-- Functions for interacting with the Player.
--

---@class Player
local Player = package.core["Player"]

local Serializer = require 'Serializer'
local Event = require 'Event'
local Game = require 'Game'
local utils = require 'utils'
local Legal = require 'Legal'

Player.record = {}
Player.record_old = {}

-- container for crimes committed
local CrimeRecord = utils.inherits(nil, "CrimeRecord")

function CrimeRecord.New(rec)
	rec = rec or {}
	setmetatable(rec, CrimeRecord.meta)
	rec.fine = 0
	rec.crimetype = {}
  return rec
end


function CrimeRecord:Add(crime, fine)

	if not fine then
		fine = Legal.CrimeType[crime].basefine
	end

	-- if first time for this crime type
	if not self.crimetype[crime] then
		self.crimetype[crime] = {}
		self.crimetype[crime].count = 0
	end

	self.crimetype[crime].count = self.crimetype[crime].count +1
	self.fine = self.fine + fine
	return self.fine
end


function CrimeRecord:SetFine(fine)
	self.fine = fine
	return self.fine
end

-- add two crime records together
function CrimeRecord:Append(record)
	self.fine = self.fine + record.fine

	for k,v in pairs(record.crimetype) do
		for i = 1, record.crimetype[k].count do
			self:Add(k, 0)
		end
	end
end


function CrimeRecord:Serialize()
	local tmp = CrimeRecord.Super().Serialize(self)
	local ret = {}
	for k,v in pairs(tmp) do
		ret[k] = v
	end
	return ret
end


function CrimeRecord.Unserialize(data)
	local obj = CrimeRecord.Super().Unserialize(data)
	setmetatable(obj, CrimeRecord.meta)
	return obj
end


--
-- Method: AddCrime
--
-- Add a crime to the player's criminal record
--
-- > Game.player:AddCrime(crime, fine, faction)
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
--
-- Availability:
--
--   2015 August
--
-- Status:
--
--   experimental
--
function Player:AddCrime (crime, fine, faction)
	local forFaction = (faction and faction.id) or Game.system.faction.id

	-- first time for this faction
	if not self.record[forFaction] then
		self.record[forFaction] = CrimeRecord.New()
	end

	self.record[forFaction]:Add(crime, fine)
end


local function __GetCrime (record)

	-- return crime record and total fine for faction
	local listOfCrime, fine

	if Game.player.flightState == "HYPERSPACE" then
		-- no crime in hyperspace
		listOfCrime = {}
		fine = 0
	elseif not record then
		-- first time for this faction, clean record
		listOfCrime = {}
		fine = 0
	else
		listOfCrime = record.crimetype
		fine = record.fine
	end

	return listOfCrime, fine
end

--
-- Method: GetCrimeRecord
--
-- Get players past criminal record of and total payed fine for faction
--
-- > criminalRecord, fine = Game.player:GetCrimeRecord()
-- > criminalRecord, fine = Game.player:GetCrimeRecord(faction)
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
--
-- Availability:
--
--   2015 September
--
-- Status:
--
--   experimental
--
function Player:GetCrimeRecord (faction)
	local forFaction = (faction and faction.id) or Game.system.faction.id
	return __GetCrime(self.record_old[forFaction])
end


--
-- Method: GetCrimeOutstanding
--
-- Get players current outstanding criminal record and total unpayed fine for faction
--
-- > criminalRecord, fine = Game.player:GetCrimeOutstanding()
-- > criminalRecord, fine = Game.player:GetCrimeOutstanding(faction)
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
--
-- Availability:
--
--   2015 September
--
-- Status:
--
--   experimental
--
function Player:GetCrimeOutstanding (faction)
	local forFaction = (faction and faction.id) or Game.system.faction.id
	return __GetCrime(self.record[forFaction])
end



--
-- Method: ClearCrimeFine
--
-- Clear the current record of outstanding fines and crimes for player
--
-- > Game.player:ClearCrimeFine()
-- > Game.player:ClearCrimeFine(faction)
-- > Game.player:ClearCrimeFine(faction, clean)
--
-- Parameters:
--
--   faction - optional argument, defaults to the faction id of the system
--             the player is currently in
--
--   clean - optional Boolean argument, defaults to false, resulting in the
--           cleared fines to still be moved to the player's crime record
--           over past offences
--
--
-- Availability:
--
--   2015 September
--
-- Status:
--
--   experimental
--
function Player:ClearCrimeFine (faction, forget)
	local forFaction = (faction and faction.id) or Game.system.faction.id

	self.record[forFaction].fine = 0		 -- Clear fine

	if not forget then
		if not self.record_old[forFaction] then
			self.record_old[forFaction] = CrimeRecord.New()
		end
		self.record_old[forFaction]:Append(self.record[forFaction], true)
	end

	self.record[forFaction] = nil			 -- Clear record
end

--
-- Method: ClearCrimeRecordHistory
--
-- Clear the player's crime record history, excluding current unpayed offences
--
-- > Game.player:ClearCrimeRecordHistory()
-- > Game.player:ClearCrimeRecordHistory(faction)
--
-- Parameters:
--
--   faction - optional argument, defaults to the faction if of the system
--             the player is currently in
--
--
-- Availability:
--
--   2015 September
--
-- Status:
--
--   experimental
--
function Player:ClearCrimeRecordHistory (faction)
	local forFaction = (faction and faction.id) or Game.system.faction.id
	self.record_old[forFaction] = nil
end

--
-- Method: GetLegalStatus
--
-- > local legal_status = Game.player:GetLegalStatus(faction)
--
-- Parameters:
--
--   faction - optional argument, defaults to the faction player is in.
--
--   legal_status - one of strings "CLEAN", "OFFENDER", "CRIMINAL", "OUTLAW",
--	                or "FUGITIVE", mapping to translated strings in lang/ui-core
--
function Player:GetLegalStatus (faction)
	local crimes, fine = self:GetCrimeOutstanding()
	self:setprop("fine", fine)

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

--
-- Method: GetMoney
--
-- Get the player's current money
--
-- > money = player:GetMoney()
--
-- Return:
--
--   money - the player's money, in dollars
--
-- Availability:
--
--   alpha 10
--
-- Status:
--
--   experimental
--
function Player:GetMoney ()
	return self.cash
end

--
-- Method: SetMoney
--
-- Set the player's money
--
-- > player:SetMoney(money)
--
-- Parameters:
--
--   money - the new amount of money, in dollars
--
-- Availability:
--
--   alpha 10
--
-- Status:
--
--   experimental
--
function Player:SetMoney (m)
	self:setprop("cash", m)
end

--
-- Method: AddMoney
--
-- Add an amount to the player's money
--
-- > money = player:AddMoney(change)
--
-- Parameters:
--
--   change - the amount of money to add to the player's money, in dollars
--
-- Return:
--
--   money - the player's new money, in dollars
--
-- Availability:
--
--   alpha 10
--
-- Status:
--
--   experimental
--
function Player:AddMoney (m)
	self:setprop("cash", self.cash + m)
end


local loaded_data

local onGameStart = function ()

	-- Don't reset these in onGameEnd (where they belong), since that
	-- sometimes clears out the data before autosave-exit can get to it
	-- (call order for event triggers is arbitrary)...
	Player.record = {}
	Player.record_old = {}

	if (loaded_data) then
		Game.player:setprop("cash", loaded_data.cash)

		-- ...thus we need to manually unserialize them
		Player.record = loaded_data.record
		Player.record_old = loaded_data.record_old

		loaded_data = nil
	end
end

local serialize = function ()

	local data = {
		cash = Game.player.cash,
		record = Game.player.record,
		record_old = Game.player.record_old
	}

	return data
end


local unserialize = function (data)
	loaded_data = data
	Player.cash = data.cash
	Player.record = data.record
	Player.record_old = data.record_old
end

local onGameEnd = function ()
	-- clean up for next game:
end

Event.Register("onGameEnd", onGameEnd)
Event.Register("onGameStart", onGameStart)
Serializer:RegisterClass("CrimeRecord", CrimeRecord)
Serializer:Register("Player", serialize, unserialize)

return Player
