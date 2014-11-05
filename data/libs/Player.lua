-- Copyright © 2008-2015 Pioneer Developers. See AUTHORS.txt for details
-- Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

local Player = import_core("Player")
local Serializer = import("Serializer")
local Event = import("Event")
local Game = import("Game")
local utils = import("utils")
local Legal = import("Legal")

Player.record = {}

local CrimeRecord = {} -- the table representing the class, which will double as the metatable for the instances
CrimeRecord.__index = CrimeRecord -- failed table lookups on the instances should fallback to the class table, to get methods

function CrimeRecord.New()
  local self = setmetatable({}, CrimeRecord)
  self.fine = 0
  self.crimetype = {}
  return self
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
--   faction - optional argument, defaults to the system the player is in
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

	-- todo: could have systemPath as faction for independent?
end

--
-- Method: GetCrime
--
-- Get criminal record and total fine for player for faction
--
-- > criminalrecord, fine = Game.player:GetCrime(faction)
--
-- Parameters:
--
--   faction - optional argument, defaults to the system the player is in
--
-- Return:
--
--   criminalRecord - a table with count and fine for each crime committed
--
--   fine - the total fine of the player in faction
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
function Player:GetCrime (faction)
	local forFaction = (faction and faction.id) or Game.system.faction.id

	-- return crime record and total fine for faction
	local listOfCrime, fine

	if Game.player.flightState == "HYPERSPACE" then
		-- no crime in hyperspace
		listOfCrime = {}
		fine = 0
	elseif not self.record[forFaction] then
		-- first time for this faction, clean record
		listOfCrime = {}
		fine = 0
	else
		listOfCrime = self.record[forFaction].crimetype
		fine = self.record[forFaction].fine
	end

	return listOfCrime, fine
end



--
-- Method: ClearCrimeFine
--
-- Clear the crime record for player
--
-- > Game.player:ClearCrimeFine(faction)
--
-- Parameters:
--
--   faction - optional argument, defaults to the system the player is in
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
function Player:ClearCrimeFine (faction)
	local forFaction = (faction and faction.id) or Game.system.faction.id

	-- TODO: what about paying fine but keeping record?

	self.record[forFaction] = nil
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
	if (loaded_data) then
		Game.player:setprop("cash", loaded_data.cash)
		Player.record = loaded_data.record
		loaded_data = nil
	end
end

local serialize = function ()

	local data = {
		cash = Game.player.cash,
		record = Game.player.record,
	}

	return data
end


local unserialize = function (data)
	loaded_data = data
	Player.cash = data.cash
	Player.record = data.record
end

local onGameEnd = function ()
	-- clean up for next game:
	Player.record = {}
end

Event.Register("onGameEnd", onGameEnd)
Event.Register("onGameStart", onGameStart)
Serializer:Register("Player", serialize, unserialize)

return Player
