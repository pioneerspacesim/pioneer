-- Copyright © 2008-2019 Pioneer Developers. See AUTHORS.txt for details
-- Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

local Engine = import("Engine")
local Rand = import("Rand")
local Lang = import("Lang")
local Game = import("Game")
local Event = import("Event")
local Serializer = import("Serializer")

local l = Lang.GetResource("module-easteregg-message")
local max_flavour_index = 1

local flavours = {}
for i = 0,max_flavour_index do
	table.insert(flavours, {
		title = l["FLAVOUR_" .. i .. "_TITLE"],
		body  = l["FLAVOUR_" .. i .. "_BODY"],
	})
end

local ads = {}

local onChat = function (form, ref, option)
	local ad = ads[ref]

	form:Clear()

	if option == -1 then
		form:Close()
		return
	end

	form:SetTitle(flavours[ad.flavour].title)
	form:SetMessage("\n" .. flavours[ad.flavour].body)
end

local onDelete = function (ref)
	ads[ref] = nil
end

local makeAdvert = function (station, flavour_index)

	local ad = {
		flavour = flavour_index,
		station = station,
	}

	ad.desc = flavours[ad.flavour].title
	local ref = station:AddAdvert({
		description = ad.desc,
--		icon        = "lightning",
		onDelete    = onDelete,
		onChat      = onChat})
	ads[ref] = ad

--	print("Hatched egg:\t".. station.label .. flavours[ad.flavour].title)

end


local onCreateBB = function (station)

	-- deterministically generate our instance
	local rand = Rand.New(station.seed + 1)

	-- How many stations for each advert?
	local one_in_n = 50

	if rand:Number(0, one_in_n) < 1 then
		makeAdvert(station, rand:Integer(1,#flavours))
	end
end

local loaded_data

local onGameStart = function ()
	ads = {}

	if not loaded_data or not loaded_data.ads then return end

	for k,ad in pairs(loaded_data.ads) do
		local ref = ad.station:AddAdvert({
			description = ad.desc,
--			icon        = "lightning",
			onDelete    = onDelete,
			onChat      = onChat,
		})
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

Serializer:Register("Message", serialize, unserialize)
