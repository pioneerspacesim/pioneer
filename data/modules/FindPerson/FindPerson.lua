-- Copyright © 2008-2026 Pioneer Developers. See AUTHORS.txt for details
-- Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

local Engine = require 'Engine'
local Lang = require 'Lang'
local Game = require 'Game'
local Space = require 'Space'
local Comms = require 'Comms'
local Event = require 'Event'
local Timer = require 'Timer'
local Mission = require 'Mission'
local Passengers = require 'Passengers'
local Format = require 'Format'
local Serializer = require 'Serializer'
local Character = require 'Character'
local NameGen = require 'NameGen'
local Ship = require 'Ship'
local utils = require 'utils'
local PlayerState = require 'PlayerState'

local MissionUtils = require 'modules.MissionUtils'
local ShipBuilder = require 'modules.MissionUtils.ShipBuilder'

local l = Lang.GetResource 'module-findperson'
local lc = Lang.GetResource 'core'

local PirateTemplate = MissionUtils.ShipTemplates.GenericPirate
local MercenaryTemplate = MissionUtils.ShipTemplates.GenericMercenary

-- Mission framework conditions
local max_mission_dist = 20
local typical_reward = 50

local flavours = {
	{ id = "DELIVER_MESSAGE", ship = false, taxi = false, company = false, max_risk = 0.1 },
	{ id = "TRAITOR", ship = false, taxi = false, company = true, max_risk = 1 },
	{ id = "PRIVATEER", ship = true, taxi = false, company = false, max_risk = 0.75 },
	{ id = "DELIVER_DOCUMENT", ship = true, taxi = false, company = true, max_risk = 1 },
	{ id = "TAXI", ship = false, taxi = true, company = false, max_risk = 0.1 },
	{ id = "EVACUATION", ship = false, taxi = true, company = true, max_risk = 1 },
}

local ads = {}
local missions = {}

local isQualifiedFor = function(reputation, ad)
	return reputation >= 16 or
		reputation >= 8 and ad.risk < 0.8 and ad.urgency < 0.8 or
		reputation >= 4 and ad.risk < 0.4 and ad.urgency < 0.4 or
		reputation >= 2 and ad.risk < 0.2 and ad.urgency < 0.2 or
		false
end

-- Returns the number of flavours of the given string (assuming first flavour has suffix '_1').
local getNumberOfFlavours = function (str)
	local num = 1

	while l:get(str .. "_" .. num) do
		num = num + 1
	end
	return num - 1
end

local getRiskMsg = function (mission)
	local gender = (mission.wanted.female and "_FEMALE" or "_MALE")
	return l:get("RISK_" .. mission.flavour.id .. gender .. "_" .. math.ceil(mission.risk * getNumberOfFlavours("RISK_" .. mission.flavour.id .. gender)))
		or l["RISK" .. gender .. "_" .. math.ceil(mission.risk * getNumberOfFlavours("RISK" .. gender))]
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
		form:SetMessage(l["DENY_" .. Engine.rand:Integer(1, getNumberOfFlavours("DENY"))])
		return
	end

	form:AddNavButton(ad.location)

	if option == 0 then
		local introtext = string.interp(ad.introtext, {
			client   = ad.client.name,
			wanted   = ad.wanted.name,
			company  = ad.company,
			cash     = Format.Money(ad.reward, false),
			system   = ad.location:GetStarSystem().name,
			sectorx  = ad.location.sectorX,
			sectory  = ad.location.sectorY,
			sectorz  = ad.location.sectorZ,
			shipid   = ad.shipid,
			domicile = ad.domicile:GetSystemBody().name,
			dist     = string.format("%.2f", ad.dist),
		})
		form:SetMessage(introtext)

	elseif option == 1 then
		local gender = ad.wanted.female and "_FEMALE" or "_MALE"
		form:SetMessage(l["HOW_TO_" .. ad.flavour.id .. gender])

	elseif option == 2 then
		form:SetMessage(string.interp(getRiskMsg(ad), { wanted = ad.wanted.name }))

	elseif option == 3 then
		form:SetMessage(string.interp(l["HOW_MUCH_TIME_" .. ad.flavour.id], { date = Format.Date(ad.due) }))

	elseif option == 4 then
		if ad.flavour.taxi and Passengers.CountFreeBerths(Game.player) == 0 then
			form:SetMessage(l.YOU_DO_NOT_HAVE_A_CABIN)
			form:RemoveNavButton()
			return
		end
		form:RemoveAdvertOnClose()
		ads[ref] = nil
		local mission = {
			type        = "FindPerson",
			client      = ad.client,
			wanted      = ad.wanted,
			company     = ad.company,
			location    = ad.location,
			destination = ad.location:SystemOnly(),
			visited     = {},
			shipid      = ad.shipid,
			ship        = nil,
			interceptor = nil,
			domicile    = ad.domicile,
			introtext   = ad.introtext,
			flavour     = ad.flavour,
			risk        = ad.risk,
			reward      = ad.reward,
			due         = ad.due,
		}
		table.insert(missions, Mission.New(mission))
		mission.wanted:Save() -- make the character available for other scripts
		mission.wanted.lastSavedSystemPath = ad.location
		form:SetMessage(l["ACCEPTED_" .. ad.flavour.id])
		return
	end

	form:AddOption(l["HOW_CAN_I_FIND_" .. (ad.wanted.female and "HER" or "HIM")], 1)
	form:AddOption(l.IS_THERE_A_RISK, 2)
	form:AddOption(l.HOW_MUCH_TIME, 3)
	form:AddOption(l.REPEAT_THE_REQUEST, 0)
	form:AddOption(l.OK_AGREED, 4)
end

local onDelete = function (ref)
	ads[ref] = nil
end

local isEnabled = function (ref)
	return ads[ref] ~= nil and isQualifiedFor(Character.persistent.player.reputation, ads[ref])
end

local placeAdvert = function (station, ad)
	local desc = string.interp(l["ADTEXT_" .. Engine.rand:Integer(1, getNumberOfFlavours("ADTEXT"))], {
		system = ad.location:GetStarSystem().name,
		cash   = Format.Money(ad.reward, false),
	})

	local ref = station:AddAdvert({
		title       = l["ADTITLE_" .. Engine.rand:Integer(1, getNumberOfFlavours("ADTITLE"))],
		description = desc,
		icon        = ad.flavour.taxi and "taxi" or "delivery",
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
	if nearbysystems == nil then
		nearbysystems = MissionUtils.GetNearbyStationPaths(Game.system, max_mission_dist, function (s) return #s:GetStationPaths() > 2 end)
	end
	if #nearbysystems == 0 then return end
	local location = nearbysystems[Engine.rand:Integer(1, #nearbysystems)]
	local dist = location:DistanceTo(Game.system)

	local flavour = flavours[Engine.rand:Integer(1, #flavours)]
	local risk = Engine.rand:Number(0.01, flavour.max_risk)
	local urgency = Engine.rand:Number(1)
	local ns = location:GetStarSystem().numberOfStations
	local reward = math.ceil(dist * (typical_reward + ns) * (1 + risk) * (1.5 + urgency) * Engine.rand:Number(0.8, 1.2))
	local due = Game.time + ns * 86400 + MissionUtils.TravelTime(dist) * 1.75 * (1.5 - urgency) * Engine.rand:Number(0.9, 1.1)

	local female = Engine.rand:Integer(1) == 1
	local introtext = "INTROTEXT_" .. flavour.id .. (female and "_FEMALE" or "_MALE")

	local ad = {
		station   = station,
		domicile  = station.path,
		introtext = l[introtext .. "_" .. Engine.rand:Integer(1, getNumberOfFlavours(introtext))],
		flavour   = flavour,
		client    = Character.New(),
		wanted    = Character.New({ female = female }),
		company   = flavour.company and string.interp(l["COMPANY_" .. Engine.rand:Integer(1, getNumberOfFlavours("COMPANY"))], { name = NameGen.Surname() }) or nil,
		location  = location,
		shipid    = flavour.ship and Ship.MakeRandomLabel() or nil,
		dist      = dist,
		due       = utils.round(due, 3600),
		risk      = risk,
		urgency   = urgency,
		reward    = utils.round(reward, 100),
	}

	placeAdvert(station, ad)
end

local onCreateBB = function (station)
	local num = Engine.rand:Integer(math.ceil(Game.system.population / 6))
	for _ = 1, num do
		makeAdvert(station)
	end
end

local onUpdateBB = function (station)
	for ref, ad in pairs(ads) do
		if ad.due < Game.time + 5*24*60*60 then -- five day timeout
			ad.station:RemoveAdvert(ref)
		end
	end
	if Engine.rand:Integer(10*24*60*60) < 60*60 then -- roughly once every ten days
		makeAdvert(station)
	end
end

local onShipFiring = function (ship)
	if ship:IsPlayer() then return end

	for ref, mission in pairs(missions) do
		if mission.interceptor == ship and not mission.surprise then
			local greeting = "MERCENARY_GREETING_" .. (mission.wanted.female and "FEMALE" or "MALE")
			local msg = string.interp(l[greeting .. "_" .. Engine.rand:Integer(1, getNumberOfFlavours(greeting))], { wanted = mission.wanted.name })
			Comms.ImportantMessage(msg, ship.label)
			mission.surprise = true
		end
	end
end

local onShipHit = function (ship, attacker)
	if ship:IsPlayer() then return end
	if attacker == nil then return end

	if attacker:isa('Ship') then
		for ref, mission in pairs(missions) do
			if mission.ship == ship then
				ship:AIKill(attacker)
				break
			end
		end
	end
end

local onShipDestroyed = function (ship, attacker)
	if ship:IsPlayer() then return end

	for ref, mission in pairs(missions) do
		if mission.ship == ship then
			mission.ship = nil
			break
		end
		if mission.interceptor == ship then
			mission.interceptor = nil
			break
		end
	end
end

local onFrameChanged = function (player)
	if not player:isa("Ship") or not player:IsPlayer() then return end

	for ref, mission in pairs(missions) do
		if mission.location:IsSameSystem(Game.system.path) and player.frameBody
			and player.frameBody.path == Space.GetBody(mission.location:GetSystemBody().parent.index).path
			and not mission.flavour.ship and not mission.interceptor then

			local riskmargin = Engine.rand:Number(-0.3, 0.0) -- Add some random luck
			if (mission.risk + riskmargin) > Engine.rand:Number(1) then
				-- The wanted person or one of his/her enemies has hired a mercenary to intercept
				local threat = 10.0 + mission.risk * 25.0
				local ship = ShipBuilder.MakeShipDocked(Space.GetBody(mission.location.bodyIndex), MercenaryTemplate, threat)
				mission.interceptor = ship
				if mission.location.type == "STARPORT_SURFACE" then
					ship:AIEnterLowOrbit(Space.GetBody(mission.location:GetSystemBody().parent.index))
				end
				Timer:CallAt(Game.time + 5, function () ship:AIKill(player) end)
			end
		end

		if mission.status == "ACTIVE" and Game.time > mission.due then
			mission.status = "FAILED"
		end
	end
end

local onEnterSystem = function (player)
	for ref, mission in pairs(missions) do
		if mission.location:IsSameSystem(Game.system.path) and mission.status == "ACTIVE" and mission.flavour.ship then

			local ship, pirate_msg
			local threat = 10.0 + mission.risk * 25.0
			local riskmargin = Engine.rand:Number(-0.3, 0.3) -- Add some random luck

			if (mission.risk + riskmargin) > Engine.rand:Number(1) then
				ship = ShipBuilder.MakeShipNear(player, PirateTemplate, threat, 50, 100)
				ship:SetLabel(mission.shipid)
				pirate_msg = string.interp(l["PIRATE_GREETING_" .. Engine.rand:Integer(1, getNumberOfFlavours("PIRATE_GREETING"))], { client = mission.client.name })
				Comms.ImportantMessage(pirate_msg, ship.label)
				Comms.ImportantMessage(string.interp(l.TRANSMITTING_MSG, { shipid = mission.shipid }))
				Timer:CallAt(Game.time + 5, function ()
					riskmargin = Engine.rand:Number(-0.3, 0.3)
					if (mission.risk + riskmargin) > Engine.rand:Number(1) then
						pirate_msg = string.interp(l["PIRATE_TAUNTS_" .. Engine.rand:Integer(1, getNumberOfFlavours("PIRATE_TAUNTS"))], { client = mission.client.name })
						Comms.ImportantMessage(pirate_msg, ship.label)
						mission.surprise = true
						ship:FireMissileAt("any", player)
						ship:AIKill(player)
					else
						pirate_msg = string.interp(l["PIRATE_ANSWER_" .. Engine.rand:Integer(1, getNumberOfFlavours("PIRATE_ANSWER"))], { client = mission.client.name })
						Comms.ImportantMessage(pirate_msg, ship.label)
						ship:AIDockWith(Space.GetBody(mission.location.bodyIndex))
					end
					mission.destination = mission.domicile
					mission.status = "PENDING_RETURN"
				end)
			else
				ship = ShipBuilder.MakeShipDocked(Space.GetBody(mission.location.bodyIndex), PirateTemplate, threat)
				ship:SetLabel(mission.shipid)
			end
			mission.ship = ship
		end

		if mission.status == "ACTIVE" and Game.time > mission.due then
			mission.status = "FAILED"
		end
	end
end

local onPlayerDocked = function (player, station)
	for ref, mission in pairs(missions) do
		local msg

		if mission.interceptor then
			if station.type == "STARPORT_SURFACE" then
				mission.interceptor:AIEnterLowOrbit(Space.GetBody(station:GetSystemBody().parent.index))
			else
				mission.interceptor:AIFlyTo(station)
			end
		end

		if mission.status == "PENDING_RETURN" then
			if mission.domicile == station.path then
				local reputation = 2
				local oldReputation = Character.persistent.player.reputation
				if Game.time <= mission.due then
					if mission.surprise then
						if mission.flavour.ship then
							msg = string.interp(l["SUCCESS_ATK_" .. Engine.rand:Integer(1, getNumberOfFlavours("SUCCESS_ATK"))], { wanted = mission.wanted.name })
						else
							msg = string.interp(l["SUCCESS_INT_" .. Engine.rand:Integer(1, getNumberOfFlavours("SUCCESS_INT"))], { wanted = mission.wanted.name })
						end
					else
						msg = string.interp(l["SUCCESS_MSG_" .. Engine.rand:Integer(1, getNumberOfFlavours("SUCCESS_MSG"))], { wanted = mission.wanted.name })
					end
					Comms.ImportantMessage(msg, mission.client.name)
					Character.persistent.player.reputation = Character.persistent.player.reputation + reputation
					PlayerState.AddMoney(mission.reward)
				else
					msg = string.interp(l["FAILUREMSG_" .. Engine.rand:Integer(1, getNumberOfFlavours("FAILUREMSG"))], { wanted = mission.wanted.name })
					Comms.ImportantMessage(msg, mission.client.name)
					Character.persistent.player.reputation = Character.persistent.player.reputation - reputation
				end
				if mission.flavour.taxi then
					Passengers.DisembarkPassenger(player, mission.wanted)
				end
				Event.Queue("onReputationChanged", oldReputation, Character.persistent.player.killcount,
					Character.persistent.player.reputation, Character.persistent.player.killcount)
				mission.wanted:UnSave() -- remove character from the persistent characters table
				mission:Remove()
				missions[ref] = nil
			end
		else
			if mission.location == station.path then
				msg = string.interp(l["GREETING_" .. mission.flavour.id], { client = mission.client.name })
				Comms.ImportantMessage(msg, mission.wanted.name)
				if mission.flavour.taxi then
					if Passengers.CountFreeBerths(player) > 0 then
						Passengers.EmbarkPassenger(player, mission.wanted)
						mission.destination = mission.domicile
						mission.status = "PENDING_RETURN"
					else
						-- cabin occupied or player has removed cabin?
						Comms.ImportantMessage(l.YOU_DO_NOT_HAVE_A_CABIN, mission.wanted.name)
					end
				else
					mission.destination = mission.domicile
					mission.status = "PENDING_RETURN"
				end
			else
				-- do nothing if not in the right system or a tipster was already there
				if mission.location:IsSameSystem(Game.system.path) and not mission.tipster then
					-- don't increase probability if last station is current station
					if station.path ~= mission.visited[#mission.visited] then
						table.insert(mission.visited, station.path)
					end
					if #mission.visited > Engine.rand:Number(4) then
						local tipster = Character.New()
						local tip = "TIP_" .. (mission.wanted.female and "FEMALE" or "MALE")
						msg = string.interp(l[tip .. "_" .. Engine.rand:Integer(1, getNumberOfFlavours(tip))], { wanted = mission.wanted.name, station = mission.location:GetSystemBody().name })
						Comms.ImportantMessage(msg, tipster.name)
						mission.tipster = true
					end
				end
			end
		end
	end
end

local onPlayerUndocked = function (player, station)
	for ref, mission in pairs(missions) do
		if mission.interceptor then
			mission.interceptor:AIKill(player)
		end
	end
end

local onLeaveSystem = function (ship)
	nearbysystems = nil
	for ref, mission in pairs(missions) do
		mission.ship = nil
		mission.interceptor = nil
	end
end

local onReputationChanged = function (oldRep, oldKills, newRep, newKills)
	for ref, ad in pairs(ads) do
		local oldQualified = isQualifiedFor(oldRep, ad)
		if isQualifiedFor(newRep, ad) ~= oldQualified then
			Event.Queue("onAdvertChanged", ad.station, ref);
		end
	end
end

local loaded_data

local onGameStart = function ()
	if loaded_data and loaded_data.ads then
		ads = {}
		missions = {}

		for _, ad in pairs(loaded_data.ads) do
			placeAdvert(ad.station, ad)
		end
		missions = loaded_data.missions
		loaded_data = nil
	end
end

local onGameEnd = function ()
	nearbysystems = nil
end

local buildMissionDescription = function (mission)
	local ui = require 'pigui'
	local desc = {}
	local dist = Game.system and string.format("%.2f", Game.system:DistanceTo(mission.location:SystemOnly())) or "???"
	local domicileDist = Game.system and string.format("%.2f", Game.system:DistanceTo(mission.domicile)) or "???"
	local danger = getRiskMsg(mission)

	desc.description = mission.introtext:interp({
		client = mission.client.name,
		wanted = mission.wanted.name,
		company = mission.company,
		system = mission.location:GetStarSystem().name,
		sectorx = mission.location.sectorX,
		sectory = mission.location.sectorY,
		sectorz = mission.location.sectorZ,
		shipid = mission.shipid,
		domicile = mission.domicile:GetSystemBody().name,
		cash = ui.Format.Money(mission.reward, false),
		dist = dist
	})

	desc.location = mission.location:SystemOnly()
	desc.client = mission.client
	desc.returnLocation = mission.domicile

	desc.details = {
		{ l.WANTED, mission.wanted.name },
		{ l.SYSTEM, ui.Format.SystemPath(mission.location) },
		{ l.DISTANCE, dist .. " " .. lc.UNIT_LY },
		mission.flavour.ship and { l.SHIP, mission.shipid },
		false,
		{ l.CLIENT, mission.client.name },
		{ l.SYSTEM, ui.Format.SystemPath(mission.domicile) },
		{ l.SPACEPORT, mission.domicile:GetSystemBody().name },
		{ l.DISTANCE, domicileDist .. " " .. lc.UNIT_LY },
		mission.flavour.company and { l.COMPANY, mission.company },
		false,
		{ l.DEADLINE, ui.Format.Date(mission.due) },
		{ l.DANGER, string.interp(danger, { wanted = mission.wanted.name }) },
	}

	return desc
end

local serialize = function ()
	return { ads = ads, missions = missions }
end

local unserialize = function (data)
	loaded_data = data
end

Event.Register("onCreateBB", onCreateBB)
Event.Register("onUpdateBB", onUpdateBB)
Event.Register("onFrameChanged", onFrameChanged)
Event.Register("onEnterSystem", onEnterSystem)
Event.Register("onLeaveSystem", onLeaveSystem)
Event.Register("onPlayerDocked", onPlayerDocked)
Event.Register("onPlayerUndocked", onPlayerUndocked)
Event.Register("onShipFiring", onShipFiring)
Event.Register("onShipHit", onShipHit)
Event.Register("onShipDestroyed", onShipDestroyed)
Event.Register("onGameStart", onGameStart)
Event.Register("onGameEnd", onGameEnd)
Event.Register("onReputationChanged", onReputationChanged)

Mission.RegisterType("FindPerson", l.FIND_PERSON, buildMissionDescription)

Serializer:Register("FindPerson", serialize, unserialize)
