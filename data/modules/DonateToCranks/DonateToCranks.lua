-- Copyright © 2008-2020 Pioneer Developers. See AUTHORS.txt for details
-- Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

local Engine = require 'Engine'
local Lang = require 'Lang'
local Game = require 'Game'
local Character = require 'Character'
local Event = require 'Event'
local Serializer = require 'Serializer'
local Format = require 'Format'

local l = Lang.GetResource("module-donatetocranks")

local flavours = {}
for i = 0,5 do
	table.insert(flavours, {
		title     = l["FLAVOUR_" .. i .. "_TITLE"],
		message   = l["FLAVOUR_" .. i .. "_MESSAGE"],
	})
end

local ads = {}


local addReputation = function (money)
	local curRep = Character.persistent.player.reputation
	local newRep

	if curRep >= 1 then
		local exp = math.log(money)/math.log(10) - (math.log(curRep)/math.log(2) - 1)
		newRep = curRep + 2^exp
	else
		newRep = curRep + 2^(math.log(money)/math.log(10))
	end
	Character.persistent.player.reputation = newRep
end


local onChat = function (form, ref, option)
	local ad = ads[ref]
	form:Clear()

	form:SetTitle(ad.title)
	form:SetFace(ad.charcter)

	if option == -1 then
		form:Close()
		return
	end

	if option == 0 then
		form:SetMessage(ad.message)
	elseif Game.player:GetMoney() < option then
		form:SetMessage(l.YOU_DO_NOT_HAVE_ENOUGH_MONEY)
	else
		if option >= 10000 then
			form:SetMessage(l.WOW_THAT_WAS_VERY_GENEROUS)
		else
			form:SetMessage(l.THANK_YOU_ALL_DONATIONS_ARE_WELCOME)
		end
		Game.player:AddMoney(-option)
		addReputation(option * ad.modifier)
	end

	-- Draw buttons donation button options
	for i=0,5,1 do
		donate = math.floor(10^i)
		form:AddOption(Format.Money(donate, false), donate)
	end
end

local onDelete = function (ref)
	ads[ref] = nil
end

local onCreateBB = function (station)
	local n = Engine.rand:Integer(1, #flavours)

	local ad = {
		modifier = n == 6 and 1.5 or 1.0, -- donating to FOSS is twice as good
		title    = flavours[n].title,
		message  = flavours[n].message,
		station  = station,
		charcter = Character.New({armour=false}),
	}

	local ref = station:AddAdvert({
		description = ad.title,
		icon        = "donate_to_cranks",
		onChat      = onChat,
		onDelete    = onDelete})
	ads[ref] = ad
end

local loaded_data

local onGameStart = function ()
	ads = {}

	if not loaded_data or not loaded_data.ads then return end

	for k,ad in pairs(loaded_data.ads) do
		local ref = ad.station:AddAdvert({
			description = ad.title,
			icon        = "donate_to_cranks",
			onChat      = onChat,
			onDelete    = onDelete})
		ads[ref] = ad
	end

	loaded_data = nil
end

local serialize = function ()
	return { ads = ads }
end

local unserialize = function (data)
	loaded_data = data
end

Event.Register("onCreateBB", onCreateBB)
Event.Register("onGameStart", onGameStart)

Serializer:Register("DonateToCranks", serialize, unserialize)
