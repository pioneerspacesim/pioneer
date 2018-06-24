-- Copyright © 2008-2018 Pioneer Developers. See AUTHORS.txt for details
-- Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

local Engine = import("Engine")
local Lang = import("Lang")
local Game = import("Game")
local Space = import("Space")
local Comms = import("Comms")
local Timer = import("Timer")
local Event = import("Event")
local Mission = import("Mission")
local NameGen = import("NameGen")
local Character = import("Character")
local Format = import("Format")
local Serializer = import("Serializer")
local ShipDef = import("ShipDef")
local Ship = import("Ship")
local Rand = import("Rand")
local utils = import("utils")

local InfoFace = import("ui/InfoFace")
local NavButton = import("ui/NavButton")

local l = Lang.GetResource("module-racing")

-- Get the UI class
local ui = Engine.ui

-- don't produce missions for further than this many light years away
local max_race_dist = 15
local racing_probability = 0.2

local flavours = {}
for i = 0,3 do
	table.insert(flavours, {
		adtext      = l["FLAVOUR_" .. i .. "_ADTEXT"],
		introtext   = l["FLAVOUR_" .. i .. "_INTROTEXT"],
		successmsg  = l["FLAVOUR_" .. i .. "_SUCCESSMSG"],
		failuremsg  = l["FLAVOUR_" .. i .. "_FAILUREMSG"],
	})
end

local ads = {}
local missions = {}
local records = {}

local onDelete = function (ref)
	ads[ref] = nil
end

local isEnabled = function (ref)
	return true
end

local onChat = function (form, ref, option)
	local ad = ads[ref]

	form:Clear()

	if option == -1 then
		form:Close()
		return
	end

	form:AddNavButton(ad.location)

	if option == 0 then
		local sys = ad.location:GetStarSystem()
		local sbody = ad.location:GetSystemBody()

		local introtext = string.interp(flavours[ad.flavour].introtext, {
			system    = sys.name,
			spaceport = sbody.name,
			sectorX   = ad.location.sectorX,
			sectorY   = ad.location.sectorY,
			sectorZ   = ad.location.sectorZ,
			cash      = Format.Money(ad.reward,false),
			record    = Format.Duration(ad.record),
			fee       = Format.Money(ad.participation,false),
		})
		form:SetMessage(introtext)

	elseif option == 1 then
		if Game.player:GetMoney() < ad.participation then
			form:SetMessage(l.YOU_DONT_HAVE_ENOUGH_MONEY)
			return
		end

		Game.player:AddMoney(-ad.participation)
		form:RemoveAdvertOnClose()

		ads[ref] = nil

		local mission = {
			type		= "Racing",
			startlocation	= ad.startlocation,
			location	= ad.location,
			reward		= ad.reward,
			start		= Game.time,
			due		= ad.due,
			flavour         = ad.flavour,
			record		= ad.record,
			status		= 'ACTIVE',
		}

		table.insert(missions,Mission.New(mission))

		form:SetMessage(l.EXCELLENT)

		return
	end
	form:AddOption(l.OK_AGREED, 1);
end

local nearbysystems
local makeAdvert = function (station)
	if nearbysystems == nil then
		nearbysystems = Game.system:GetNearbySystems(max_race_dist, function (s) return #s:GetStationPaths() > 0 end)
	end
	if #nearbysystems == 0 then return end
	local flavour = Engine.rand:Integer(1, #flavours)
	local rand = Rand.New(#nearbysystems * station.path:GetSystemBody().index)
	local nearbysystem = nearbysystems[rand:Integer(1,#nearbysystems)]
	local nearbystations = nearbysystem:GetStationPaths()
	local rand2 = Rand.New(#nearbystations * #nearbysystems * station.path:GetSystemBody().index)
	local location = nearbystations[rand2:Integer(1,#nearbystations)]
	local dist = location:DistanceTo(Game.system)
	local reward = Engine.rand:Number(1000, 20000)
	local participation = reward / 25 + Engine.rand:Number(10,200)
	local due = Game.time + Engine.rand:Number(15*60*60*24, 42*60*60*24)
	local advert_record = 0
	local client = Character.New()
	local startlocation = station.path
	reward = math.ceil(reward)
	participation = math.ceil(participation)

	for ref,items in pairs(records) do
		if items.location == location and items.startlocation == startlocation then
			advert_record = items.record
			break
		end
	end

	if advert_record == 0 then
		table.insert(records, 1, {
			     record = Engine.rand:Number(9*60*60*24, 30*60*60*24),
			     location = location,
			     startlocation = startlocation,
			     })

		advert_record = records[1].record
	end

	local ad = {
		flavour = flavour,
		startlocation = startlocation,
		location = location,
		dist = dist,
		reward = reward,
		record = advert_record,
		station = station,
		due = due,
		client = client,
		participation = participation,
	}

	ad.desc = string.interp(flavours[ad.flavour].adtext, {
		system	= nearbysystem.name,
	})
	local ref = station:AddAdvert({
		description = ad.desc,
		icon        = "racing",
		onChat      = onChat,
		onDelete    = onDelete,
		isEnabled   = isEnabled})
	ads[ref] = ad
end

local onCreateBB = function (station)
	local num = Engine.rand:Number(0, 1)
	if num < racing_probability then
		makeAdvert(station)
	end
end

local onShipDocked = function (ship, station)
	if not ship:IsPlayer() then return end
	for ref,mission in pairs(missions) do
		local new_record = Game.time - mission.start
		local old_record = 0
		local rref = 0
		for record_ref,record in pairs(records) do
			if record.location == mission.location and record.startlocation == mission.startlocation then
				old_record = record.record
				rref = record_ref
			end
		end
		if new_record > old_record then
			mission.status = 'FAILED'
			Comms.ImportantMessage(flavours[mission.flavour].failuremsg, mission.client.name)
			mission:Remove()
			missions[ref] = nil
		elseif mission.location == station.path then
			Comms.ImportantMessage(flavours[mission.flavour].successmsg, mission.client.name)
			ship:AddMoney(mission.reward)
			Character.persistent.player.reputation = Character.persistent.player.reputation + 3
			records[rref].record = new_record
			mission:Remove()
			missions[ref] = nil
		end
	end
end

local onUpdateBB = function (station)
	for ref,ad in pairs(ads) do
		if (ad.due < Game.time + 5*60*60*24) then
			ad.station:RemoveAdvert(ref)
		end
	end
	if Engine.rand:Integer(62*24*60*60) < 60*60 and #ads < 1 then -- roughly once every 2 months
		makeAdvert(station)
	end
end

local loaded_data

local onGameStart = function ()
	ads = {}
	missions = {}
	records = {}

	if not loaded_data or not loaded_data.ads then return end

	for k,ad in pairs(loaded_data.ads) do
		local ref = ad.station:AddAdvert({
			description = ad.desc,
			icon        = "racing",
			onChat      = onChat,
			onDelete    = onDelete,
			isEnabled   = isEnabled})
		ads[ref] = ad
	end

	missions = loaded_data.missions
	records = loaded_data.records

	loaded_data = nil
end

local onGameEnd = function ()
	nearbysystems = nil
	missions = nil
	records = nil
end

local onClick = function (mission)
	return ui:Grid(2,1)
		:SetColumn(0,{ui:VBox(10):PackEnd({
							ui:Margin(10),
							ui:Grid(2,1)
								:SetColumn(0, {
									ui:VBox():PackEnd({
										ui:Label(l.RECORD)
									})
								})
								:SetColumn(1, {
									ui:VBox():PackEnd({
										ui:MultiLineText(Format.Duration(mission.record))
									})
								}),
							ui:Grid(2,1)
								:SetColumn(0, {
									ui:VBox():PackEnd({
										ui:Label(l.REWARD)
									})
								})
								:SetColumn(1, {
									ui:VBox():PackEnd({
										ui:MultiLineText(Format.Money(mission.reward))
									})
								}),
							ui:Grid(2,1)
								:SetColumn(0, {
									ui:VBox():PackEnd({
										ui:Label(l.STARTING_SPACEPORT)
									})
								})
								:SetColumn(1, {
									ui:VBox():PackEnd({
										ui:MultiLineText(mission.startlocation:GetSystemBody().name)
									})
								}),
							ui:Grid(2,1)
								:SetColumn(0, {
									ui:VBox():PackEnd({
										ui:Label(l.STARTING_SYSTEM)
									})
								})
								:SetColumn(1, {
									ui:VBox():PackEnd({
										ui:MultiLineText(mission.startlocation:GetStarSystem().name.." ("..mission.startlocation.sectorX..","..mission.startlocation.sectorY..","..mission.startlocation.sectorZ..")")
									})
								}),
							ui:Grid(2,1)
								:SetColumn(0, {
									ui:VBox():PackEnd({
										ui:Label(l.FINAL_SPACEPORT)
									})
								})
								:SetColumn(1, {
									ui:VBox():PackEnd({
										ui:MultiLineText(mission.location:GetSystemBody().name)
									})
								}),
							ui:Grid(2,1)
								:SetColumn(0, {
									ui:VBox():PackEnd({
										ui:Label(l.FINAL_SYSTEM)
									})
								})
								:SetColumn(1, {
									ui:VBox():PackEnd({
										ui:MultiLineText(mission.location:GetStarSystem().name.." ("..mission.location.sectorX..","..mission.location.sectorY..","..mission.location.sectorZ..")")
									})
								}),
							NavButton.New(l.SET_AS_TARGET, mission.location),
		})})
		:SetColumn(1, {
			ui:VBox(10):PackEnd(InfoFace.New(mission.client))
		})
end

local serialize = function ()
	return { ads = ads, missions = missions, records = records }
end

local unserialize = function (data)
	loaded_data = data
end

Event.Register("onCreateBB", onCreateBB)
Event.Register("onGameStart", onGameStart)
Event.Register("onShipDocked", onShipDocked)
Event.Register("onUpdateBB", onUpdateBB)
Event.Register("onGameEnd", onGameEnd)

Mission.RegisterType('Racing',l.RACING,onClick)

Serializer:Register("Racing", serialize, unserialize)
