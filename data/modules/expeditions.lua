-- Copyright © 2008-2013 Pioneer Developers. See AUTHORS.txt for details
-- Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

-- This is a mission script for long-range expeditions to unexplored systems.  There will be a shorter range, common, version; and a long-range, rare version.  Both versions will be rare
-- compared to delivery missions and such.  The missions will be difficult and will require the player to own a large ship.
--
--
--
--
--
--
-- Get the translator function
local t = Translate:GetTranslator()
-- Get the UI class
local ui = Engine.ui

-- don't produce missions for further than this many light years away
local max_expedition_dist = 100
-- typical time for travel to a system max_expedition_dist away
local typical_travel_time = 2.4 * max_expedition_dist * 24 * 60 * 60
-- typical reward for delivery to a system max_expedition_dist away
local typical_reward = 225 * max_expedition_dist


local ads = {}
local missions = {}


local add_passengers = function (group)
  Game.player:RemoveEquip('UNOCCUPIED_CABIN', group)
	Game.player:AddEquip('PASSENGER_CABIN', group)
	passengers = passengers + group
end

local remove_passengers = function (group)
	Game.player:RemoveEquip('PASSENGER_CABIN', group)
	Game.player:AddEquip('UNOCCUPIED_CABIN', group)
	passengers = passengers - group
end

local onChat = function (form, ref, option)
	local expedition_flavours = Translate:GetFlavours('Expedition')
	local ad = ads[ref]
	
		form:Clear()

	if option == -1 then
		form:Close()
		return
	end
	
	
	if option == 0 then
		form:SetFace(ad.client)

		local  systems = Game.system:GetNearbySystems(100, function (s) return Game.system.explored == false end) 
		local sys   = ad.location:GetStarSystem()
		local sbody = ad.location:GetSystemBody()
		
		
		local introtext = string.interp(expedition_flavours[ad.flavour].introtext, {
			name     = ad.client.name,
			cash     = Format.Money(ad.reward),
			system   = sys.name,
			group  = ad.group,
			sectorx  = ad.location.sectorX,
			sectory  = ad.location.sectorY,
			sectorz  = ad.location.sectorZ,
			dist     = string.format("%.2f", ad.dist),
		})

		form:SetMessage(introtext)
		
	elseif option == 1 then
		

	elseif option == 2 then
				form:SetMessage(expedition_flavours[ad.flavour].expeditiontext) 

	elseif option == 3 then
		local capacity = Game.player:GetEquipSlotCapacity('CABIN')
		local Playerstats = Game.player:ship:GetStats()
		if capacity < ad.group or Game.player:GetEquipCount('CABIN', 'UNOCCUPIED_CABIN') < ad.group then
			form:SetMessage(t("You do not have enough cabin space on your ship."))
			form:AddOption(t('HANG_UP'), -1)
		elseif Playerstats.totalMass < 120 then
			form:SetMessage(t("You expect us to take you seriously flying that puddle jumper? Please come back when you own a ship larger than 120t."))
			return
		end

		add_passengers(ad.group)

		form:RemoveAdvertOnClose()

		ads[ref] = nil

		local mission = {
			type	 = "Expedition",
			client	 = ad.client,
			start    = ad.station.path,
			location = ad.location,
			risk	 = ad.risk,
			reward	 = ad.reward,
			due	 = ad.due,
			group	 = ad.group,
			flavour	 = ad.flavour
		}

		table.insert(missions,Mission.New(mission))

		form:SetMessage(t("Excellent."))
		form:AddOption(t('HANG_UP'), -1)

		return
	elseif option == 4 then
		if expedition_flavours[ad.flavour].single == 1 then
			form:SetMessage(t("We estimate we will return by  ")..Format.Date(ad.due))
		else
			form:SetMessage(t("We need to return with the data by ")..Format.Date(ad.due))
		end

	elseif option == 5 then
		form:SetMessage(expedition_flavours[ad.flavour].danger)
	end
	
	elseif option == 6 then
		form:SetMessage(

	form:AddOption(t("?"), 1)
	form:AddOption(t("What kind of expedition are we talking about?"), 2)
	form:AddOption(t("When will we arrive back?"), 4)
	form:AddOption(t("Will there be any danger?"), 5)
	form:AddOption(t("Could you repeat the original request?"), 0)
	form:AddOption(t("Ok, agreed."), 3)
	form:AddOption(t('HANG_UP'), -1)
end


local onDelete = function (ref)
	ads[ref] = nil
end

local unexploredsystems
local makeAdvert = function (station)
	local reward, due, location, unexploredsystem, dist
	local expedition_flavours = Translate:GetFlavours('Expedition')
	local client = Character.New({title = expedition_flavours[flavour].scientist})
	local flavour = Engine.rand:Integer(1,#expedition_flavours)
	local urgency = expedition_flavours[flavour].urgency
	local risk = expedition_flavours[flavour].risk

	if expedition_flavours[flavour].localdelivery == 1 then
		nearbysystem = Game.system
		local nearbystations = Game.system:GetStationPaths()
		location = nearbystations[Engine.rand:Integer(1,#nearbystations)]
		if location ==  station.path then return end
		local locdist = Space.GetBody(location.bodyIndex)
		dist = station:DistanceTo(locdist)
		if dist < 1000 then return end
		reward = 25 + (math.sqrt(dist) / 15000) * (1+urgency)
		due = Game.time + ((4*24*60*60) * (Engine.rand:Number(1.5,3.5) - urgency))
	else
		if nearbysystems == nil then
			nearbysystems = Game.system:GetNearbySystems(max_delivery_dist, function (s) return #s:GetStationPaths() > 0 end)
		end
		if #nearbysystems == 0 then return end
		nearbysystem = nearbysystems[Engine.rand:Integer(1,#nearbysystems)]
		dist = nearbysystem:DistanceTo(Game.system)
		local nearbystations = nearbysystem:GetStationPaths()
		location = nearbystations[Engine.rand:Integer(1,#nearbystations)]
		reward = ((dist / max_delivery_dist) * typical_reward * (1+risk) * (1.5+urgency) * Engine.rand:Number(0.8,1.2))
		due = Game.time + ((dist / max_delivery_dist) * typical_travel_time * (1.5-urgency) * Engine.rand:Number(0.9,1.1))
	end

	local ad = {
		station		= station,
		flavour		= flavour,
		client		= client,
		location	= location,
		dist            = dist,
		due		= due,
		risk		= risk,
		urgency		= urgency,
		reward		= reward,
		isfemale	= isfemale,
		faceseed	= Engine.rand:Integer(),
	}

	local sbody = ad.location:GetSystemBody()

	ad.desc = string.interp(delivery_flavours[flavour].adtext, {
		system	= nearbysystem.name,
		cash	= Format.Money(ad.reward),
		starport = sbody.name,
	})

	local ref = station:AddAdvert(ad.desc, onChat, onDelete)
	ads[ref] = ad
end

local onCreateBB = function (station)
	local num = Engine.rand:Integer(0, math.ceil(Game.system.population))
	for i = 1,num do
		makeAdvert(station)
	end
end

local onUpdateBB = function (station)
	local delivery_flavours = Translate:GetFlavours('DeliverPackage')
	for ref,ad in pairs(ads) do
		if delivery_flavours[ad.flavour].localdelivery == 0
			and ad.due < Game.time + 5*60*60*24 then -- five day timeout for inter-system
			ad.station:RemoveAdvert(ref)
		elseif delivery_flavours[ad.flavour].localdelivery == 1
			and ad.due < Game.time + 2*60*60*24 then -- two day timeout for locals
			ad.station:RemoveAdvert(ref)
		end
	end
	if Engine.rand:Integer(12*60*60) < 60*60 then -- roughly once every twelve hours
		makeAdvert(station)
	end
end
