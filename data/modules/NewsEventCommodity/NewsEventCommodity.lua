-- Copyright © 2008-2024 Pioneer Developers. See AUTHORS.txt for details
-- Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

-- Create a news event on the BBS (to do: place it on
-- StationView/Lobby.lua board?) with information which will indicate
-- (indirectly) a change in demand of item.


-- copy table by value, rather than the default: by reference.
local copyTable = function(T)
	local t2 = {}
	for k,v in pairs(T) do
		t2[k] = v
	end
	return t2
end

local Comms = require 'Comms'
local debugView = require 'pigui.views.debug'
local ui = require 'pigui'
local Economy = require 'Economy'
local Engine = require 'Engine'
local Lang = require 'Lang'
local Game = require 'Game'
local Event = require 'Event'
local Format = require 'Format'
local Serializer = require 'Serializer'
local Commodities = require 'Commodities'

local l = Lang.GetResource("module-newseventcommodity")

local maxDist = 50          -- max distance for spawning news (ly)
local minTime = 15768000    -- no news the first 6 months of a new game (sec)

-- to spawn a new event per hyperjump, provided no other news.
local eventProbability = 1/20

-- don't allow too many news at the same time:
local maxNumberNews = 3

-- max index of flavoured variants
local maxIndexOfIndNewspapers = 10
local maxIndexOfAdTitles = 3
local maxIndexOfTitles = 4
local maxIndexOfGreetings = 5

local flavours = {
	{                                           -- flavour 0 in en.json
		cargo = Commodities.medicines.name, -- which commodity is affected
		multiplier = 4,                             -- change in price (and stock)
	}, {
		cargo = Commodities.battle_weapons.name,       --1
		multiplier = 4,
	}, {
		cargo = Commodities.grain.name,                --2
		multiplier = 10,
	}, {
		cargo = Commodities.fruit_and_veg.name,        --3
		multiplier = 6,
	}, {
		cargo = Commodities.narcotics.name,            --4
		multiplier = -4,
	}, {
		cargo = Commodities.slaves.name,               --5
		multiplier = 7,
	}, {
		cargo = Commodities.liquor.name,               --6
		multiplier = 3,
	}, {
		cargo = Commodities.industrial_machinery.name, --7
		multiplier = 6,
	}, {
		cargo = Commodities.mining_machinery.name,     --8
		multiplier = 6,
	}, {
		cargo = Commodities.live_animals.name,         --9
		multiplier = 3,
	}, {
		cargo = Commodities.air_processors.name,       --10
		multiplier = 5,
	}, {
		cargo = Commodities.animal_meat.name,          --11
		multiplier = 3,
	}, {
		cargo = Commodities.computers.name,            --12
		multiplier = 3,
	}, {
		cargo = Commodities.robots.name,               --13
		multiplier = -4,
	}, {
		cargo = Commodities.plastics.name,             --14
		multiplier = 3,
	}, {
		cargo = Commodities.narcotics.name,            --15
		multiplier = 4,
	}, {
		cargo = Commodities.farm_machinery.name,       --16
		multiplier = 5,
	}, {
		cargo = Commodities.metal_ore.name,            --17
		multiplier = -10,
	}, {
		cargo = Commodities.consumer_goods.name,       --18
		multiplier = 3,
	}, {
		cargo = Commodities.precious_metals.name,      --19
		multiplier = -3,
	}, {
		cargo = Commodities.fertilizer.name,           --20
		multiplier = -3,
	}, {
		cargo = Commodities.nerve_gas.name,            --21
		multiplier = -4,
	}, {
		cargo = Commodities.hand_weapons.name,         --22
		multiplier = 3,
	}, {
		cargo = Commodities.metal_alloys.name,         --23
		multiplier = 3,
	}
}


-- add strings to flavours
for i = 1,#flavours do
	local f = flavours[i]
	f.headline = l["FLAVOUR_" .. i-1 .. "_HEADLINE"]
	f.newsbody = l["FLAVOUR_" .. i-1 .. "_NEWSBODY"]
end

-- will hold the ads of the current system
local ads = {}

-- will hold the current news for all systems
local news = {}

-- print ad to BBS
local onChat = function (form, ref, option)
	local ad = ads[ref]

	form:Clear()

	local faction = ad.n.syspath:GetStarSystem().faction.name

	local newspaper
	if faction == "Solar Federation" then
		newspaper = l.NEWSPAPER_FED
	elseif faction == "Commonwealth of Independent Worlds" then
		newspaper = l.NEWSPAPER_CIW
	elseif faction == "Empire" then
		newspaper = l.NEWSPAPER_IMP
	else
		newspaper = l["NEWSPAPER_IND_"..Engine.rand:Integer(0,maxIndexOfIndNewspapers)]
	end

	local title = string.interp(l["TITLE_"..Engine.rand:Integer(0,maxIndexOfTitles)] , {
		newspaper = newspaper,
	})

	form:SetTitle(title)

	local newsbody = string.interp(flavours[ad.n.flavour].newsbody, {
		system   = ad.n.syspath:GetStarSystem().name,
		sectorx  = ad.n.syspath.sectorX,
		sectory  = ad.n.syspath.sectorY,
		sectorz  = ad.n.syspath.sectorZ,
	})

	form:SetMessage(newsbody)
end


-- if the advert is removed, (RemoveAdvert, or hyperspace)
local onDelete = function (ref)
	ads[ref] = nil
end


-- is a table of systems, news only travel so far (defined by maxDist)
local nearbySystems

local createNewsEvent = function (timeInHyper)

	-- publication date for the news shown on the BBS, i.e. some time
	-- during players time in hyperspace
	local date = Game.time - Engine.rand:Number(0.1,0.9)*timeInHyper

	-- find system for event, excluding the current one
	if nearbySystems == nil then
		local dist = maxDist  * Engine.rand:Number(0.4,1.0)
		nearbySystems = Game.system:GetNearbySystems(dist, function (s) return #s:GetStationPaths() > 0 end)
	end

	-- scrap news if no systems with stations
	if #nearbySystems == 0 then return end

	local maxChecks, i = 3, 0
	local flavour, cargo, system

	-- make maxChecks attempts to find a flavour whose commodity is
	-- not illegal in all systems in nearbySystems
	repeat
		i = i + 1
		flavour = Engine.rand:Integer(1, #flavours)
		cargo = flavours[flavour].cargo
		local candidateSystems = copyTable(nearbySystems)
		repeat
			local index = Engine.rand:Integer(1, #candidateSystems)
			system = candidateSystems[index]

			if system:IsCommodityLegal(cargo) then
				--print("cargo,", cargo, "is legal in:", system.name)
			else
				--print("cargo,", cargo, "is legal in:", system.name)
				system = nil
			end
			table.remove(candidateSystems, index)
		until system ~= nil or #candidateSystems == 0

	until system ~= nil or maxChecks == i

	-- no system with the commodity found, abort.
	if system == nil then return end

	-- expiration time depends on distance to event
	local distance = system:DistanceTo(Game.system)  -- from here to there
	local hyperTime = 6200*distance^2                -- hyperspace time to get there
	local inSystemTime = 5*86400                     -- in system time to get there
	local expires = Game.time + hyperTime*Engine.rand:Number(0.9,1.6) + inSystemTime

	local newsEvent = {
		syspath = system.path,   -- SystemPath object of the news event
		flavour	= flavour,  -- id-number of flavour
		cargo   = cargo,    -- relevant cargo
		expires = expires,  -- expiration date of NEWS in seconds
		date    = date,     -- timestamp, i.e. "publication date"
		multiplier  = flavours[flavour].multiplier,
	}

	-- add headline from flavour, and more info to be displayed
	newsEvent.description = string.interp(flavours[flavour].headline, {
		system = system.name,
		cargo = Commodities[cargo]:GetName(),
		-- Turn string "23:09:27 3 Jan 3200" into "3 Jan 3200:"
		date  = Format.DateOnly(date),
	})
	table.insert(news, newsEvent)

	print(string.format("NEWS created: #news = %s", #news))
end


-- go through news table and remove any expired entry
local checkOldNews = function ()
	for i,n in pairs(news) do
		if n.expires < Game.time then
			table.remove(news, i)
		end
	end
end


-- check if we should remove any ads
local checkAdvertsRemove = function(station)
	for ref,ad in pairs(ads) do
		if ad.n.expires < Game.time then
			ad.station:RemoveAdvert(ref)
		end
	end
end

-- check if we should add any ads to the BBS of the station
local checkAdvertsAdd = function(station)

	-- a system path that points to the current star system
	local currentSystem = Game.system.path

	-- if there are any news, go through them
	for i,n in pairs(news) do
		-- don't place ad if we're in the system of the event
		if not currentSystem:IsSameSystem(n.syspath) then
			local ref = station:AddAdvert({
				title       = l["ADTITLE_"..Engine.rand:Integer(0,maxIndexOfAdTitles)],
				description = n.description,
				icon        = "news",
				onChat      = onChat,
				onDelete    = onDelete
			})
			ads[ref] = {n=n, station=station}
		end
	end
end


-- when we enter system, create all ads we need on all BBS
local onCreateBB = function (station)

	-- create ads
	checkAdvertsAdd(station)

end


local timeInHyperspace

local onEnterSystem = function (player)
	if (not player:IsPlayer()) then return end

	-- remove old news before making new
	checkOldNews()

	timeInHyperspace = Game.time - timeInHyperspace

	-- create a news event with low probability
	if Engine.rand:Number(0,1) < eventProbability and
		#news < maxNumberNews and Game.GetStartTime() > minTime then
		createNewsEvent(timeInHyperspace)
	end

	return
end


local onLeaveSystem = function (ship)
	if not ship:IsPlayer() then return end
	nearbySystems = nil
	timeInHyperspace = Game.time
end

local cache = nil

local onShipDocked = function (ship, station)
	if not ship:IsPlayer() then return end

	-- remove expired news from the news table
	checkOldNews()

	-- remove ads for news that have expired
	checkAdvertsRemove(station)

	local currentSystem = Game.system.path

	-- if there are any news,
	for i,n in pairs(news) do
		-- if this is the system of the news
		if currentSystem:IsSameSystem(n.syspath) then
			-- send a grateful greeting from the station if the player cargo is right
			local cargo_item = Commodities[n.cargo]

			if ship:GetComponent('CargoManager'):CountCommodity(cargo_item) > 0 and n.multiplier > 0 then
				local greeting = string.interp(l["GRATEFUL_GREETING_"..Engine.rand:Integer(0,maxIndexOfGreetings)],
					{cargo = cargo_item:GetName()})
				Comms.Message(greeting)
			end

			local price = station:GetCommodityPrice(cargo_item)
			local flow, affinity = Economy.GetStationFlowParams(station, cargo_item)
			local stock, demand = Economy.GetCommodityStockFromFlow(cargo_item, flow, affinity)

			-- Store original stats for commodity, to reset to later
			cache = {commodity = cargo_item, price=price, stock = stock, demand = demand}

			-- How close to finish? Linear decrease from 1.0 at start -> 0 at end of event
			local progress = (n.expires - Game.time) / (n.expires - n.date)

			local newPrice, newStock, newDemand
			if n.multiplier > 0 then
				newPrice = n.multiplier * price                             -- increase price
				newStock = 0                                                -- remove all stock
				newDemand = demand * math.ceil(1 + progress*n.multiplier)   -- demand
			elseif n.multiplier < 0 then
				newPrice = math.ceil(price / (1 + math.abs(n.multiplier)))  -- dump price
				newStock = math.ceil(math.abs(n.multiplier * stock))        -- spam stock
				newDemand = 0                                               -- station does not buy it
			end
			-- print("--- NewsEvent old:", cargo_item:GetName(), "price:", price, "stock:", stock, "demand:", demand)
			-- print("--- NewsEvent new:", cargo_item:GetName(), "price:", newPrice, "stock:", newStock, "demand", newDemand)

			station:SetCommodityPrice(cargo_item, newPrice)
			station:SetCommodityStock(cargo_item, newStock, newDemand)
		end
	end
end


-- Reset when we leave the station. With current implementation of
-- economy, it would gradually have returned to equilibrium in 12
-- weeks time.
local onShipUndocked = function (ship, station)
	if not ship:IsPlayer() then return end

	if cache then
		station:SetCommodityPrice(cache.commodity, cache.price)
		station:SetCommodityStock(cache.commodity, cache.stock, cache.demand)

		-- print("Reset price, stock, demand: ")
		-- local price = station:GetCommodityPrice(cache.commodity)
		-- print("\tprice:", price)
		-- local flow, affinity = Economy.GetStationFlowParams(station, cache.commodity)
		-- local stock, demand = Economy.GetCommodityStockFromFlow(cache.commodity, flow, affinity)
		-- print("\tstock", stock)
		-- print("\tdemand", demand)
		cache = nil
	end
end


local loadedData

-- restore news events if anything saved, restore ads on the BBS and
local onGameStart = function ()
	ads = {}
	news = {}

	if not loadedData or not loadedData.ads then return end

	for k,ad in pairs(loadedData.ads) do
		local ref = ad.station:AddAdvert({
			title       = l["ADTITLE_" .. Engine.rand:Integer(maxIndexOfAdTitles)],
			description = ad.n.description,
			icon        = "news",
			onChat      = onChat,
			onDelete    = onDelete
		})
		ads[ref] = ad
	end

	news = loadedData.news

	loadedData = nil
end


-- at the moment, Lua virtual machine is not reset between games,
-- we must clean up after us.
local onGameEnd = function ()
	nearbySystems = nil
end

local serialize = function ()
	return { ads = ads, news = news }
end


local unserialize = function (data)
	loadedData = data
end

Event.Register("onCreateBB", onCreateBB)
Event.Register("onEnterSystem", onEnterSystem)
Event.Register("onLeaveSystem", onLeaveSystem)
Event.Register("onShipDocked", onShipDocked)
Event.Register("onShipUndocked", onShipUndocked)
Event.Register("onGameStart", onGameStart)
Event.Register("onGameEnd", onGameEnd)

Serializer:Register("NewsEventCommodity", serialize, unserialize)


debugView.registerTab("news", {
	label = "News",
	icon = ui.theme.icons.info,
	show = function() return Game.player end,
	draw = function ()
		ui.textWrapped("Note: Display of News on the BBS will not update until docking at new station.")
		if ui.button("Make News", Vector2(100, 0)) then
			createNewsEvent(0)
		end
		ui.sameLine()
		ui.text("Radius: " .. maxDist .. " ly.")

		ui.textWrapped("There are usually no News before: " .. Format.DateOnly(minTime) .." and max simultaneous news events: " .. maxNumberNews)

		for i ,n in pairs(news) do
			local system_name = n.syspath:GetStarSystem().name
			local commodity_name = Commodities[n.cargo]:GetName()

			local distance = 0
			if Game.system then
				distance = n.syspath:DistanceTo(Game.system)
			end

			local header = commodity_name .. "\t" .. system_name .. "\t" .. string.format("%.2f", distance) .. " ly"

			if ui.collapsingHeader(header, {}) then
				local headline = string.interp(
					flavours[n.flavour].headline, {
						system = system_name,
						cargo = commodity_name,
						date  = Format.DateOnly(n.date),
				})
				ui.textWrapped(headline)
				ui.spacing()

				local newsbody = string.interp(
					flavours[n.flavour].newsbody,
					{
						system   = n.syspath:GetStarSystem().name,
						sectorx  = n.syspath.sectorX,
						sectory  = n.syspath.sectorY,
						sectorz  = n.syspath.sectorZ,
				})
				ui.textWrapped(newsbody)

				ui.text("Flavour idx: " .. n.flavour)
				-- Be compatible with saves before 2024-07-10 version:
				if n.multiplier then
					ui.text("Price multiplier: " .. n.multiplier .. " on cargo: " .. n.cargo)
				else
					ui.text("Price demand: " .. n.demand .. " on cargo: " .. n.cargo)
				end
				ui.text("Start: " .. Format.DateOnly(n.date))
				ui.text("End: " .. Format.DateOnly(n.expires))

				if ui.button("Delete news", Vector2(100, 0)) then
					table.remove(news, i)
				end
				ui.separator()
			end
		end
	end
})
