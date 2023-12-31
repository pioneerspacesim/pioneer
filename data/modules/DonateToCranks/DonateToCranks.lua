-- Copyright © 2008-2024 Pioneer Developers. See AUTHORS.txt for details
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
for i = 0,9 do
	table.insert(flavours, {
		title     = l["FLAVOUR_" .. i .. "_ADTITLE"],
		desc      = l["FLAVOUR_" .. i .. "_DESC"],
		message   = l["FLAVOUR_" .. i .. "_MESSAGE"],
	})
end

local ads = {}


local addReputation = function (money, character)
	local char = character or Character.persistent.player
	local curRep = char.reputation
	local newRep

	if curRep >= 1 then
		local exp = math.log(money)/math.log(10) - (math.log(curRep)/math.log(2) - 1)
		newRep = curRep + 2^exp
	else
		newRep = curRep + 2^(math.log(money)/math.log(10))
	end
	char.reputation = newRep
	return newRep
end


local computeReputation = function (ad)
	-- compute money to get to next level of reputation

	local current_reputation = 	Character.persistent.player:GetReputationRating()
	print("\n\nCURRENT:", current_reputation)

	for i=0,5,1 do
		local donate = math.floor(10^i)
		local clone = Character.persistent.player:Clone()
		addReputation(donate * ad.modifier, clone)
		local new_reputation = clone:GetReputationRating()
		if new_reputation ~= current_reputation then
			return donate
		end
	end
	-- aught not come here unless player has maxed out reputation
	return 0
end


local onChat = function (form, ref, option)
	local ad = ads[ref]
	form:Clear()

	form:SetTitle(string.interp(flavours[ad.n].desc, ad.stringVariables))
	form:SetFace(ad.character)

	if option == -1 then
		form:Close()
		return
	end

	if option == 0 then
		form:SetMessage(string.interp(flavours[ad.n].message, ad.stringVariables) .. "\n\n" .. string.interp(
			l.SALES_PITCH, {cash = Format.Money(computeReputation(ad), false)}))

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
		local donate = math.floor(10^i)
		form:AddOption(Format.Money(donate, false), donate)
	end
end

local onDelete = function (ref)
	ads[ref] = nil
end

local onCreateBB = function (station)
	local n = Engine.rand:Integer(1, #flavours)

	-- FOSS is rare, (kind of an easter egg), skip advert
	if n == 6 and Engine.rand:Integer(0, 5) > 1 then
		return
	end

	local faction = Game.system.faction.name
	local military = Game.system.faction.militaryName
	local police = Game.system.faction.policeName
	local system = Game.system.name
	local stringVariables = {SYSTEM = system, FACTION = faction, MILITARY = military, POLICE = police}
	local ad = {
		modifier = n == 6 and 1.5 or 1.0, -- donating to FOSS is twice as good
		stringVariables = stringVariables,
		station  = station,
		character = Character.New({armour=false}),
		n        = n
	}

	local ref = station:AddAdvert({
		title       = string.interp(flavours[n].title, stringVariables),
		description = string.interp(flavours[n].desc, stringVariables),
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
			title       = string.interp(flavours[ad.n].title, ad.stringVariables),
			description = string.interp(flavours[ad.n].desc, ad.stringVariables),
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
