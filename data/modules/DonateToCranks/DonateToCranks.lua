-- Copyright © 2008-2014 Pioneer Developers. See AUTHORS.txt for details
-- Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

local Engine = import("Engine")
local Lang = import("Lang")
local Game = import("Game")
local Comms = import("Comms")
local Character = import("Character")
local Event = import("Event")
local Serializer = import("Serializer")

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

	if option == 0 then
		form:Clear()

		form:SetTitle(ad.title)
		form:SetFace({ seed = ad.faceseed })
		form:SetMessage(ad.message)

		form:AddOption("$1", 1)
		form:AddOption("$10", 10)
		form:AddOption("$100", 100)
		form:AddOption("$1000", 1000)
		form:AddOption("$10000", 10000)
		form:AddOption("$100000", 100000)

		return
	end

	if option == -1 then
		form:Close()
		return
	end

	if Game.player:GetMoney() < option then
		Comms.Message(l.YOU_DO_NOT_HAVE_ENOUGH_MONEY)
	else
		if option >= 10000 then
			Comms.Message(l.WOW_THAT_WAS_VERY_GENEROUS)
		else
			Comms.Message(l.THANK_YOU_ALL_DONATIONS_ARE_WELCOME)
		end
		Game.player:AddMoney(-option)
		addReputation(option)
	end
end

local onDelete = function (ref)
	ads[ref] = nil
end

local onCreateBB = function (station)
	local n = Engine.rand:Integer(1, #flavours)

	local ad = {
		title    = flavours[n].title,
		message  = flavours[n].message,
		station  = station,
		faceseed = Engine.rand:Integer()
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

	if not loaded_data then return end

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
