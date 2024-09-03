-- Copyright © 2008-2024 Pioneer Developers. See AUTHORS.txt for details
-- Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

-- This module is a substitute for not having a propper "bar" /
-- "bartender" where a player could go and get latest rumours or
-- advice from more experienced pilots. Instead, we just put them on
-- the BBS for now.

local Lang = require 'Lang'
local Event = require 'Event'
local Rand = require 'Rand'
local Serializer = require 'Serializer'

local l = Lang.GetResource("module-advice")

-- probability to have one advice/rumour (per BBS):
local advice_probability = .2


-- There are three different versions of flavours "Rumours",
-- "Traveller's tale", and "Traveller's advice", which have fake /
-- arbitrary indices, for inflated feeling of rich universe and lore.
local rumours_num = 1
local travellers_tale_num = 1
local travellers_advice_indices = {481, -- tame black market
								   16,  -- road faster taken
								   173, -- don't get stranded
								   13,  -- it's all relative
								   5,   -- never loose money
								   121, -- read the news
								   81,  -- donate
								   23,  -- double trouble
								   52,  -- service ship
								   248, -- change faction
								   171, -- the harder the g
}

-- Hold all different types of advice/rumours available:
local flavours = {}

-- Hold the ones published on the BBS:
local ads = {}

-- add Traveller strings to flavours-table:
for i = 1,#travellers_advice_indices do
	local num = travellers_advice_indices[i]
	table.insert(flavours, {
		ID = l['TRAVELLERS_ADVICE'] .. " #" .. num,
		headline = l["ADVICE_" .. i .. "_HEADLINE"],
		bodytext = l["ADVICE_" .. i .. "_BODYTEXT"],
	})
end

-- add Traveller tale strings to flavours-table:
for i = 1,travellers_tale_num do
	table.insert(flavours, {
		ID = l['TRAVELLERS_TALE'],
		headline = l["TALE_" .. i .. "_HEADLINE"],
		bodytext = l["TALE_" .. i .. "_BODYTEXT"],
	})
end

-- add rumours to flavours-table:
for i = 1,rumours_num do
	table.insert(flavours, {
		ID = l['RUMOUR'],
		headline = l["RUMOUR_" .. i .. "_HEADLINE"],
		bodytext = l["RUMOUR_" .. i .. "_BODYTEXT"],
	})
end

-- print ad to BBS
local onChat = function (form, ref, option)
	local ad = ads[ref]
	form:Clear()
	form:SetTitle(flavours[ad.n].ID)
	form:SetMessage(flavours[ad.n].bodytext)
end

local onDelete = function (ref)
	ads[ref] = nil
end

-- when we enter a system the BBS is created and this function is called
local onCreateBB = function (station)
	local rand = Rand.New(station.seed)
	local n = rand:Integer(1, #flavours)

	local ad = {
		station = station,
		n = n
	}

	-- only create one per BBS, with advice_probability
	local ref
	if rand:Number() < advice_probability then
		ref = station:AddAdvert({
			title       = flavours[n].ID,
			description = flavours[n].headline,
			icon        = "advice",
			onChat      = onChat,
			onDelete    = onDelete})
		ads[ref] = ad
	end
end


local loaded_data

local onGameStart = function ()
	ads = {}

	if not loaded_data or not loaded_data.ads then return end

	for k,ad in pairs(loaded_data.ads or {}) do
		local ref = ad.station:AddAdvert({
			title       = flavours[ad.n].ID,
			description = flavours[ad.n].headline,
			icon        = "advice",
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

Serializer:Register("Advice", serialize, unserialize)
