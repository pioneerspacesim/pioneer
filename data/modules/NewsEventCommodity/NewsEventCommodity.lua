-- Copyright © 2008-2014 Pioneer Developers. See AUTHORS.txt for details
-- Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

-- Create a news event on the BBS (to do: place it on
-- StationView/Lobby.lua board?) with information which will indicate
-- (indirectly) a change in demand of item.

-- useful for debuging
local tableLength = function (T)
	local count = 0
	for _ in pairs(T) do
		count = count + 1
	end
	return count
end

-- copy table by value, rather than the default: by reference.
local copyTable = function(T)
	t2 = {}
	for k,v in pairs(T) do
		t2[k] = v
	end
	return t2
end

local Comms = import("Comms")
local Engine = import("Engine")
local Lang = import("Lang")
local Game = import("Game")
local Space = import("Space")
local Event = import("Event")
local Format = import("Format")
local Serializer = import("Serializer")
local utils = import("utils")
local Equipment = import ("Equipment")

local l = Lang.GetResource("module-newseventcommodity")

-- Get the UI class
local ui = Engine.ui

local maxDist = 50          -- for spawning news (ly)
local minTime = 15768000    -- no news the first 5 months of a new game (sec)

-- to spawn a new event per hyperjump, provided no other news.
local eventProbability = 1/30

-- max index of flavoured variants
local maxIndexOfIndNewspapers = 10
local maxIndexOfTitles = 4
local maxIndexOfGreetings = 5

local flavours = {
	{                                      -- flavour 0 in en.json
		cargo = Equipment.cargo.medicines, -- which commodity is affected
		demand = 4,                        -- change in price (and stock)
	}, {
		cargo = Equipment.cargo.battle_weapons,       --1
		demand = 4,
	}, {
		cargo = Equipment.cargo.grain,                --2
		demand = 10,
	}, {
		cargo = Equipment.cargo.fruit_and_veg,        --3
		demand = 6,
	}, {
		cargo = Equipment.cargo.narcotics,            --4
		demand = -4,
	}, {
		cargo = Equipment.cargo.slaves,               --5
		demand = 7,
	}, {
		cargo = Equipment.cargo.liquor,               --6
		demand = 3,
	}, {
		cargo = Equipment.cargo.industrial_machinery, --7
		demand = 6,
	}, {
		cargo = Equipment.cargo.mining_machinery,     --8
		demand = 6,
	}, {
		cargo = Equipment.cargo.live_animals,         --9
		demand = 3,
	}, {
		cargo = Equipment.cargo.air_processors,       --10
		demand = 5,
	}, {
		cargo = Equipment.cargo.animal_meat,          --11
		demand = 3,
	}, {
		cargo = Equipment.cargo.computers,            --12
		demand = 3,
	}, {
		cargo = Equipment.cargo.robots,               --13
		demand = -4,
	}, {
		cargo = Equipment.cargo.plastics,             --14
		demand = 3,
	}, {
		cargo = Equipment.cargo.narcotics,            --15
		demand = 4,
	}, {
		cargo = Equipment.cargo.farm_machinery,       --16
		demand = 5,
	}, {
		cargo = Equipment.cargo.metal_ore,            --17
		demand = -10,
	}, {
		cargo = Equipment.cargo.consumer_goods,       --18
		demand = 3,
	}, {
		cargo = Equipment.cargo.precious_metals,      --19
		demand = -3,
	}, {
		cargo = Equipment.cargo.fertilizer,           --20
		demand = -3,
	}, {
		cargo = Equipment.cargo.nerve_gas,            --21
		demand = -4,
	}, {
		cargo = Equipment.cargo.hand_weapons,         --22
		demand = 3,
	}, {
		cargo = Equipment.cargo.metal_alloys,         --23
		demand = 3,
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
	if faction == "Federation" then
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
				--print("cargo,", cargo:GetName(), "is legal in:", system.name)
			else
				--print("cargo,", cargo:GetName(), "is legal in:", system.name)
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
		demand  = flavours[flavour].demand,
	}

	-- add headline from flavour, and more info to be displayed
	newsEvent.description = string.interp(flavours[flavour].headline, {
		system = system.name,
		cargo = cargo:GetName(),
		-- Turn string "23:09:27 3 Jan 3200" into "3 Jan 3200:"
		date  = string.match(Format.Date(date), "%d+ %w+ %d+$")
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
		local len = tableLength(ads)
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
			ref = station:AddAdvert(
				{description = n.description,
				 icon = "news",
				 onChat = onChat,
				 onDelete = onDelete})
			ads[ref] = {n=n, station=station}

			local length = tableLength(ads)
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

	-- don't allow too many news at the same time:
	local maxNumberNews = 3

	timeInHyperspace = Game.time - timeInHyperspace

	-- create a news event with low probability
	if Engine.rand:Number(0,1) < eventProbability and
		#news <= maxNumberNews and Game.time > minTime then
		createNewsEvent(timeInHyperspace)
	end

	return
end


local onLeaveSystem = function (ship)
	if not ship:IsPlayer() then return end
	nearbySystems = nil
	timeInHyperspace = Game.time
end


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
			if ship:CountEquip(n.cargo, cargo) > 0 and n.demand > 0 then
				local greeting = string.interp(l["GRATEFUL_GREETING_"..Engine.rand:Integer(0,maxIndexOfGreetings)],
					{cargo = n.cargo:GetName()})
				Comms.Message(greeting, ship.label)
			end

			local price = station:GetEquipmentPrice(n.cargo)
			local stock = station:GetEquipmentStock(n.cargo)

			local newPrice, newStockChange
			if n.demand > 0 then
				newPrice = n.demand * price -- increase price
				newStockChange = -1 * stock -- remove all stock
			elseif n.demand < 0 then
				newPrice = math.ceil(price / (1 + math.abs(n.demand)))  -- dump price
				newStockChange = math.ceil(math.abs(n.demand * stock))  -- spam stock
			else
				error("demand should probably not be 0.")
			end
			-- print("cargo:", n.cargo:GetName(), "price:", newPrice, newStockChange)
			station:SetEquipmentPrice(n.cargo, newPrice)
			station:AddEquipmentStock(n.cargo, newStockChange)
		end
	end
end


local loadedData

-- restore news events if anything saved, restore ads on the BBS and
local onGameStart = function ()
	ads = {}
	news = {}

	if not loadedData then return end

	for k,ad in pairs(loadedData.ads) do
		local ref = ad.station:AddAdvert(
			{description = ad.n.description,
			 icon = "news",
			 onChat= onChat,
			 onDelete = onDelete})
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
Event.Register("onGameStart", onGameStart)
Event.Register("onGameEnd", onGameEnd)

Serializer:Register("NewsEventCommodity", serialize, unserialize)
