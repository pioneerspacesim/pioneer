-- Copyright © 2008-2024 Pioneer Developers. See AUTHORS.txt for details
-- Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

local Engine = require 'Engine'
local Lang = require 'Lang'
local Game = require 'Game'
local Space = require 'Space'
local Comms = require 'Comms'
local Event = require 'Event'
local Mission = require 'Mission'
local MissionUtils = require 'modules.MissionUtils'
local Format = require 'Format'
local Serializer = require 'Serializer'
local Character = require 'Character'
local ShipDef = require 'ShipDef'
local Ship = require 'Ship'
local eq = require 'Equipment'
local utils = require 'utils'

-- Get the language resource
local l = Lang.GetResource("module-taxi")
local lc = Lang.GetResource 'core'

-- don't produce missions for further than this many light years away
local max_taxi_dist = 40
-- typical reward for taxi service to a system max_taxi_dist away
local typical_reward = 75 * max_taxi_dist
-- max number of passengers per trip
local max_group = 10

local num_corporations = 12
local num_pirate_taunts = 4
local num_deny = 8

local flavours = {
	{
		single = false,  -- flavour 0-2 are for groups
		urgency = 0,
		risk = 0.001,
	}, {
		single = false,
		urgency = 0,
		risk = 0,
	}, {
		single = false,
		urgency = 0,
		risk = 0,
	}, {
		single = true,  -- flavour 3- are for single persons
		urgency = 0.13,
		risk = 0.73,
	}, {
		single = true,
		urgency = 0.3,
		risk = 0.02,
	}, {
		single = true,
		urgency = 0.1,
		risk = 0.05,
	}, {
		single = true,
		urgency = 0.02,
		risk = 0.07,
	}, {
		single = true,
		urgency = 0.15,
		risk = 1,
	}, {
		single = true,
		urgency = 0.5,
		risk = 0.001,
	}, {
		single = true,
		urgency = 0.85,
		risk = 0.20,
	}, {
		single = true,
		urgency = 0.9,
		risk = 0.40,
	}, {
		single = true,
		urgency = 1,
		risk = 0.31,
	}, {
		single = true,
		urgency = 0,
		risk = 0.17,
	}
}

-- add strings to flavours
for i = 1,#flavours do
	local f = flavours[i]
	f.adtitle    = l["FLAVOUR_" .. i-1 .. "_ADTITLE"]
	f.adtext     = l["FLAVOUR_" .. i-1 .. "_ADTEXT"]
	f.introtext  = l["FLAVOUR_" .. i-1 .. "_INTROTEXT"]
	f.whysomuch  = l["FLAVOUR_" .. i-1 .. "_WHYSOMUCH"]
	f.howmany    = l["FLAVOUR_" .. i-1 .. "_HOWMANY"]
	f.danger     = l["FLAVOUR_" .. i-1 .. "_DANGER"]
	f.successmsg = l["FLAVOUR_" .. i-1 .. "_SUCCESSMSG"]
	f.failuremsg = l["FLAVOUR_" .. i-1 .. "_FAILUREMSG"]
	f.wherearewe = l["FLAVOUR_" .. i-1 .. "_WHEREAREWE"]
end

local ads = {}
local missions = {}
local passengers = 0

local add_passengers = function (group)
	Game.player:RemoveEquip(eq.misc.cabin,  group)
	Game.player:AddEquip(eq.misc.cabin_occupied, group)
	passengers = passengers + group
end

local remove_passengers = function (group)
	Game.player:RemoveEquip(eq.misc.cabin_occupied,  group)
	Game.player:AddEquip(eq.misc.cabin, group)
	passengers = passengers - group
end

local isQualifiedFor = function(reputation, ad)
	return reputation >= 16 or
		(ad.risk <  0.002 and ad.urgency < 0.3 and reputation >= 0) or
		(ad.risk <  0.2   and ad.urgency < 0.5 and reputation >= 4) or
		(ad.risk <= 0.6   and ad.urgency < 0.6 and reputation >= 8) or
		false
end

local onChat = function (form, ref, option)
	local ad = ads[ref]

	form:Clear()

	if option == -1 then
		form:Close()
		return
	end

	local qualified = isQualifiedFor(Character.persistent.player.reputation, ad)

	form:SetFace(ad.client)

	if not qualified then
		local introtext = l["DENY_"..Engine.rand:Integer(1,num_deny)-1]
		form:SetMessage(introtext)
		return
	end

	form:AddNavButton(ad.location)

	if option == 0 then
		local sys   = ad.location:GetStarSystem()

		local introtext = string.interp(flavours[ad.flavour].introtext, {
			name     = ad.client.name,
			cash     = Format.Money(ad.reward,false),
			system   = sys.name,
			sectorx  = ad.location.sectorX,
			sectory  = ad.location.sectorY,
			sectorz  = ad.location.sectorZ,
			dist     = string.format("%.2f", ad.dist),
		})

		form:SetMessage(introtext)

	elseif option == 1 then
		local corporation = l["CORPORATIONS_"..Engine.rand:Integer(1,num_corporations)-1]
		local whysomuch = string.interp(flavours[ad.flavour].whysomuch, {
			corp     = corporation,
		})

		form:SetMessage(whysomuch)

	elseif option == 2 then
		local howmany = string.interp(flavours[ad.flavour].howmany, {
			group  = ad.group,
		})

		form:SetMessage(howmany)

	elseif option == 3 then
		if not Game.player.cabin_cap or Game.player.cabin_cap < ad.group then
			form:SetMessage(l.YOU_DO_NOT_HAVE_ENOUGH_CABIN_SPACE_ON_YOUR_SHIP)
			form:RemoveNavButton()
			return
		end

		add_passengers(ad.group)

		form:RemoveAdvertOnClose()

		ads[ref] = nil

		local mission = {
			type	 = "Taxi",
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

		form:SetMessage(l.EXCELLENT)

		return
	elseif option == 4 then
		if flavours[ad.flavour].single then
			form:SetMessage(l.I_MUST_BE_THERE_BEFORE..Format.Date(ad.due))
		else
			form:SetMessage(l.WE_WANT_TO_BE_THERE_BEFORE..Format.Date(ad.due))
		end

	elseif option == 5 then
		form:SetMessage(flavours[ad.flavour].danger)
	end

	form:AddOption(l.WHY_SO_MUCH_MONEY, 1)
	form:AddOption(l.HOW_MANY_OF_YOU_ARE_THERE, 2)
	form:AddOption(l.HOW_SOON_YOU_MUST_BE_THERE, 4)
	form:AddOption(l.WILL_I_BE_IN_ANY_DANGER, 5)
	form:AddOption(l.COULD_YOU_REPEAT_THE_ORIGINAL_REQUEST, 0)
	form:AddOption(l.OK_AGREED, 3)
end

local onDelete = function (ref)
	ads[ref] = nil
end

local isEnabled = function (ref)
	return ads[ref] ~= nil and isQualifiedFor(Character.persistent.player.reputation, ads[ref])
end

local placeAdvert = function (station, ad)
	local desc = string.interp(flavours[ad.flavour].adtext, {
		system	= ad.location:GetStarSystem().name,
		cash	= Format.Money(ad.reward,false),
	})

	local ref = station:AddAdvert({
		title = flavours[ad.flavour].adtitle,
		description = desc,
		icon        = ad.urgency >=  0.8 and "taxi_urgent" or "taxi",
		due         = ad.due,
		reward      = ad.reward,
		location    = ad.location,
		onChat      = onChat,
		onDelete    = onDelete,
		isEnabled   = isEnabled})
	ads[ref] = ad
end

local nearbysystems
local makeAdvert = function (station)
	local reward, due, location, timeout
	local client = Character.New()
	local flavour = Engine.rand:Integer(1,#flavours)
	local urgency = flavours[flavour].urgency
	local risk = flavours[flavour].risk
	local group = 1
	if not flavours[flavour].single then
		group = Engine.rand:Integer(2,max_group)
	end

	if nearbysystems == nil then
		nearbysystems = Game.system:GetNearbySystems(max_taxi_dist, function (s) return #s:GetStationPaths() > 0 end)
	end
	if #nearbysystems == 0 then return end
	location = nearbysystems[Engine.rand:Integer(1,#nearbysystems)]
	local dist = location:DistanceTo(Game.system)
	reward = ((dist / max_taxi_dist) * typical_reward * (group / 2) * (1+risk) * (1+3*urgency) * Engine.rand:Number(0.8,1.2))
	reward = utils.round(reward, 50)
	due = MissionUtils.TravelTime(dist) * 1.25 * (1.5-urgency) * Engine.rand:Number(0.9,1.1)
	timeout = due/2 + Game.time -- timeout after half of the travel time
	due = utils.round(due + Game.time, 900)

	local ad = {
		station		= station,
		flavour		= flavour,
		client		= client,
		location	= location.path,
		dist        = dist,
		due		    = due,
		timeout     = timeout,
		group		= group,
		risk		= risk,
		urgency		= urgency,
		reward		= reward,
		faceseed	= Engine.rand:Integer(),
	}

	placeAdvert(station, ad)
end

local onCreateBB = function (station)
	local num = Engine.rand:Integer(0, math.ceil(Game.system.population))
	for i = 1,num do
		makeAdvert(station)
	end
end

local onUpdateBB = function (station)
	for ref,ad in pairs(ads) do
		if ad.timeout < Game.time then
			ad.station:RemoveAdvert(ref)
		end
	end
	if Engine.rand:Integer(24*60*60) < 60*60 then -- roughly once every day
		makeAdvert(station)
	end
end

local onEnterSystem = function (player)
	if (not player:IsPlayer()) then return end

	local syspath = Game.system.path

	for ref,mission in pairs(missions) do

		-- Since system names are not unique, player might jump into
		-- system with right name, but wrong coordinates
		if mission.status == "ACTIVE" and not mission.location:IsSameSystem(syspath) then
			local mission_system = mission.location:GetStarSystem()
			local current_system = syspath:GetStarSystem()
			if mission_system.name == current_system.name then
				Comms.ImportantMessage(l.WRONG_SYSTEM, mission.client.name)
			end
		end

		if mission.status == "ACTIVE" and mission.location:IsSameSystem(syspath) then

			local risk = flavours[mission.flavour].risk
			local ships = 0

			local riskmargin = Engine.rand:Number(-0.3,0.3) -- Add some random luck
			if risk >= (1 + riskmargin) then ships = 3
			elseif risk >= (0.7 + riskmargin) then ships = 2
			elseif risk >= (0.5 + riskmargin) then ships = 1
			end

			if ships < 1 and risk > 0 and Engine.rand:Integer(math.ceil(1/risk)) == 1 then ships = 1 end

			local shipdefs = utils.build_array(utils.filter(function (k,def) return def.tag == 'SHIP'
				and def.hyperdriveClass > 0 and def.roles.pirate end, pairs(ShipDef)))
			if #shipdefs == 0 then return end

			local ship

			while ships > 0 do
				ships = ships-1

				if Engine.rand:Number(1) <= risk then
					local shipdef = shipdefs[Engine.rand:Integer(1,#shipdefs)]
					local default_drive = eq.hyperspace['hyperdrive_'..tostring(shipdef.hyperdriveClass)]

					local max_laser_size = shipdef.capacity - default_drive.capabilities.mass
					local laserdefs = utils.build_array(utils.filter(
						function (k,l) return l:IsValidSlot('laser_front') and l.capabilities.mass <= max_laser_size and l.l10n_key:find("PULSECANNON") end,
						pairs(eq.laser)
					))
					local laserdef = laserdefs[Engine.rand:Integer(1,#laserdefs)]

					ship = Space.SpawnShipNear(shipdef.id, Game.player, 50, 100)
					ship:SetLabel(Ship.MakeRandomLabel())
					ship:AddEquip(default_drive)
					ship:AddEquip(laserdef)
					ship:AddEquip(eq.misc.shield_generator, math.ceil(risk * 3))
					if Engine.rand:Number(2) <= risk then
						ship:AddEquip(eq.misc.laser_cooling_booster)
					end
					if Engine.rand:Number(3) <= risk then
						ship:AddEquip(eq.misc.shield_energy_booster)
					end
					ship:AIKill(Game.player)
				end
			end

			if ship then
				local pirate_greeting = string.interp(l["PIRATE_TAUNTS_"..Engine.rand:Integer(1,num_pirate_taunts)-1], { client = mission.client.name,})
				Comms.ImportantMessage(pirate_greeting, ship.label)
			end
		end

		if mission.status == "ACTIVE" and Game.time > mission.due then
			mission.status = 'FAILED'
			Comms.ImportantMessage(flavours[mission.flavour].wherearewe, mission.client.name)
		end
	end
end

local onLeaveSystem = function (ship)
	if ship:IsPlayer() then
		nearbysystems = nil
	end
end

local onShipDocked = function (player, station)
	if not player:IsPlayer() then return end

	for ref,mission in pairs(missions) do
		if mission.location == Game.system.path or Game.time > mission.due then
			local oldReputation = Character.persistent.player.reputation
			if Game.time > mission.due then
				Comms.ImportantMessage(flavours[mission.flavour].failuremsg, mission.client.name)
				Character.persistent.player.reputation = Character.persistent.player.reputation - 2
			else
				Comms.ImportantMessage(flavours[mission.flavour].successmsg, mission.client.name)
				player:AddMoney(mission.reward)
				Character.persistent.player.reputation = Character.persistent.player.reputation + 2
			end
			Event.Queue("onReputationChanged", oldReputation, Character.persistent.player.killcount,
				Character.persistent.player.reputation, Character.persistent.player.killcount)

			remove_passengers(mission.group)

			mission:Remove()
			missions[ref] = nil
		end
	end
end

local onShipUndocked = function (player, station)
	if not player:IsPlayer() then return end
	local current_passengers = Game.player:GetEquipCountOccupied("cabin")-(Game.player.cabin_cap or 0)
	if current_passengers >= passengers then return end -- nothing changed, good

	for ref,mission in pairs(missions) do
		remove_passengers(mission.group)

		Comms.ImportantMessage(l.HEY_YOU_ARE_GOING_TO_PAY_FOR_THIS, mission.client.name)
		mission:Remove()
		missions[ref] = nil
	end
end

local onReputationChanged = function (oldRep, oldKills, newRep, newKills)
	for ref,ad in pairs(ads) do
		local oldQualified = isQualifiedFor(oldRep, ad)
		if isQualifiedFor(newRep, ad) ~= oldQualified then
			Event.Queue("onAdvertChanged", ad.station, ref);
		end
	end
end

local loaded_data

local onGameStart = function ()
	ads = {}
	missions = {}
	passengers = 0

	if not loaded_data or not loaded_data.ads then return end

	for k,ad in pairs(loaded_data.ads) do
		placeAdvert(ad.station, ad)
	end

	missions = loaded_data.missions
	passengers = loaded_data.passengers

	loaded_data = nil
end

local onGameEnd = function ()
	nearbysystems = nil
end

local buildMissionDescription = function(mission)
	local ui = require 'pigui'

	local desc = {}
	local dist = Game.system and string.format("%.2f", Game.system:DistanceTo(mission.location)) or "???"

	desc.description = flavours[mission.flavour].introtext:interp({
		name   = mission.client.name,
		system = mission.location:GetStarSystem().name,
		sectorx = mission.location.sectorX,
		sectory = mission.location.sectorY,
		sectorz = mission.location.sectorZ,
		cash   = Format.Money(mission.reward,false),
		dist  = dist
	})

	desc.client = mission.client
	desc.location = mission.location

	desc.details = {
		{ l.FROM, ui.Format.SystemPath(mission.start) },
		{ l.TO, ui.Format.SystemPath(mission.location) },
		{ l.GROUP_DETAILS, string.interp(flavours[mission.flavour].howmany, {group = mission.group}) },
		{ l.DEADLINE, ui.Format.Date(mission.due) },
		{ l.DANGER, flavours[mission.flavour].danger },
		{ l.DISTANCE, dist.." "..lc.UNIT_LY }
	}

	return desc
end

local serialize = function ()
	return { ads = ads, missions = missions, passengers = passengers }
end

local unserialize = function (data)
	loaded_data = data
end

Event.Register("onCreateBB", onCreateBB)
Event.Register("onUpdateBB", onUpdateBB)
Event.Register("onEnterSystem", onEnterSystem)
Event.Register("onLeaveSystem", onLeaveSystem)
Event.Register("onShipUndocked", onShipUndocked)
Event.Register("onShipDocked", onShipDocked)
Event.Register("onGameStart", onGameStart)
Event.Register("onGameEnd", onGameEnd)
Event.Register("onReputationChanged", onReputationChanged)

Mission.RegisterType('Taxi',l.TAXI, buildMissionDescription)

Serializer:Register("Taxi", serialize, unserialize)
