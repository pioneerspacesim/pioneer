-- Copyright © 2008-2020 Pioneer Developers. See AUTHORS.txt for details
-- Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

local Engine = require 'Engine'
local Lang = require 'Lang'
local Game = require 'Game'
local Space = require 'Space'
local Comms = require 'Comms'
local Event = require 'Event'
local Timer = require 'Timer'
local Mission = require 'Mission'
local Format = require 'Format'
local Serializer = require 'Serializer'
local Character = require 'Character'
local NameGen = require 'NameGen'
local Equipment = require 'Equipment'
local ShipDef = require 'ShipDef'
local Ship = require 'Ship'
local utils = require 'utils'

local InfoFace = import("ui/InfoFace")
local NavButton = import("ui/NavButton")

local l = Lang.GetResource("module-findperson")

-- Get the UI class
local ui = Engine.ui

-- Mission framework conditions
local max_mission_dist = 20
local typical_hyperspace_time = 3.0 * 24 * 60 * 60
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

local onChat = function (form, ref, option)
	local ad = ads[ref]

	form:Clear()

	if option == -1 then
		form:Close()
		return
	end

	local qualified = isQualifiedFor(Character.persistent.player.reputation, ad)
	local gender = (ad.wanted.female and "_FEMALE" or "_MALE")

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
		form:SetMessage(l["HOW_TO_" .. ad.flavour.id .. gender])

	elseif option == 2 then
		form:SetMessage(
			string.interp(
				l:get("RISK_" .. ad.flavour.id .. gender .. "_" .. math.ceil(ad.risk * getNumberOfFlavours("RISK_" .. ad.flavour.id .. gender)))
				or l["RISK" .. gender .. "_" .. math.ceil(ad.risk * getNumberOfFlavours("RISK" .. gender))], { wanted = ad.wanted.name }
			)
		)

	elseif option == 3 then
		form:SetMessage(string.interp(l["HOW_MUCH_TIME_" .. ad.flavour.id], { date = Format.Date(ad.due) }))

	elseif option == 4 then
		if ad.flavour.taxi and Game.player:CountEquip(Equipment.misc.cabin) == 0 and Game.player:CountEquip(Equipment.misc.cabin_occupied) == 0 then
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
			location    = ad.location:GetStarSystem().path,
			destination = ad.location,
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
			halfdone    = false,
		}
		table.insert(missions, Mission.New(mission))
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

local nearbysystems

local makeAdvert = function (station)
	if nearbysystems == nil then
		nearbysystems = Game.system:GetNearbySystems(max_mission_dist, function (s) return #s:GetStationPaths() > 2 end)
	end
	if #nearbysystems == 0 then return end
	local nearbysystem = nearbysystems[Engine.rand:Integer(1, #nearbysystems)]
	local nearbystations = nearbysystem:GetStationPaths()
	local location = nearbystations[Engine.rand:Integer(1, #nearbystations)]
	local dist = location:DistanceTo(Game.system)

	local flavour = flavours[Engine.rand:Integer(1, #flavours)]
	local risk = Engine.rand:Number(0.01, flavour.max_risk)
	local urgency = Engine.rand:Number(1)
	local reward = math.ceil(dist * (typical_reward + #nearbystations) * (1 + risk) * (1.5 + urgency) * Engine.rand:Number(0.8, 1.2))
	local due = Game.time + #nearbystations * 86400 + (dist * typical_hyperspace_time * (1.5 - urgency) * Engine.rand:Number(0.9, 1.1))

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
		due       = due,
		risk      = risk,
		urgency   = urgency,
		reward    = reward,
	}

	ad.desc = string.interp(l["ADTEXT_" .. Engine.rand:Integer(1, getNumberOfFlavours("ADTEXT"))], {system = location:GetStarSystem().name, cash = Format.Money(ad.reward, false)})

	local ref = station:AddAdvert({
		description = ad.desc,
		icon        = "default",
		onChat      = onChat,
		onDelete    = onDelete,
		isEnabled   = isEnabled})
	ads[ref] = ad
end

local onCreateBB = function (station)
	local num = Engine.rand:Integer(0, math.ceil(Game.system.population) / 2 + 1)
	for i = 1, num do
		makeAdvert(station)
	end
end

local onUpdateBB = function (station)
	for ref, ad in pairs(ads) do
		if ad.due < Game.time + 5*60*60*24 then -- five day timeout
			ad.station:RemoveAdvert(ref)
		end
	end
	if Engine.rand:Integer(4*24*60*60) < 60*60 then -- roughly once every four days
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

local defineShip = function (role)
	local shipdefs = utils.build_array(
		utils.filter(
			function (k, def)
				return def.tag == 'SHIP' and def.hyperdriveClass > 0 and def.roles[role]
			end,
			pairs(ShipDef)
		)
	)
	local shipdef = shipdefs[Engine.rand:Integer(1, #shipdefs)]
	local drivedef = Equipment.hyperspace["hyperdrive_" .. shipdef.hyperdriveClass]

	local missiledefs = utils.build_array(
		utils.filter(
			function (k, l)
				return l:IsValidSlot("missile") and
					not l.l10n_key:find("UNGUIDED")
			end,
			pairs(Equipment.misc)
		)
	)
	local missiledef = missiledefs[Engine.rand:Integer(1, #missiledefs)]

	local max_laser_size = shipdef.capacity - (drivedef.capabilities.mass + missiledef.capabilities.mass)
	local laserdefs = utils.build_array(
		utils.filter(
			function (k, l)
				return l:IsValidSlot("laser_front") and
					l.capabilities.mass <= max_laser_size and
					l.l10n_key:find("PULSECANNON")
			end,
			pairs(Equipment.laser)
		)
	)
	local laserdef = laserdefs[Engine.rand:Integer(1, #laserdefs)]

	return shipdef, drivedef, laserdef, missiledef
end

local onFrameChanged = function (player)
	if not player:isa("Ship") or not player:IsPlayer() then return end

	for ref, mission in pairs(missions) do
		if mission.destination:IsSameSystem(Game.system.path) and player.frameBody
			and player.frameBody.path == Space.GetBody(mission.destination:GetSystemBody().parent.index).path
			and not mission.flavour.ship and not mission.interceptor then

			local riskmargin = Engine.rand:Number(-0.3, 0.0) -- Add some random luck
			if (mission.risk + riskmargin) > Engine.rand:Number(1) then
				-- The wanted person or one of his/her enemies has hired a mercenary to intercept
				local ship, shipdef, drivedef, laserdef, missiledef
				shipdef, drivedef, laserdef, missiledef = defineShip("mercenary")
				ship = Space.SpawnShipDocked(shipdef.id, Space.GetBody(mission.destination.bodyIndex))
				ship:SetLabel(Ship.MakeRandomLabel())
				ship:AddEquip(drivedef)
				ship:AddEquip(laserdef)
				mission.interceptor = ship
				if mission.destination.type == "STARPORT_SURFACE" then
					ship:AIEnterLowOrbit(Space.GetBody(mission.destination:GetSystemBody().parent.index))
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
	if not player:IsPlayer() then return end

	for ref, mission in pairs(missions) do
		if mission.destination:IsSameSystem(Game.system.path) and not mission.halfdone and mission.flavour.ship then

			local ship, pirate_msg, shipdef, drivedef, laserdef, missiledef
			shipdef, drivedef, laserdef, missiledef = defineShip("pirate")

			local riskmargin = Engine.rand:Number(-0.3, 0.3) -- Add some random luck
			if (mission.risk + riskmargin) > Engine.rand:Number(1) then
				ship = Space.SpawnShipNear(shipdef.id, player, 50, 100)
				ship:SetLabel(mission.shipid)
				ship:AddEquip(drivedef)
				ship:AddEquip(laserdef)
				ship:AddEquip(missiledef)
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
						ship:AIDockWith(Space.GetBody(mission.destination.bodyIndex))
					end
					mission.halfdone = true
				end)
			else
				ship = Space.SpawnShipDocked(shipdef.id, Space.GetBody(mission.destination.bodyIndex))
				ship:SetLabel(mission.shipid)
				ship:AddEquip(drivedef)
				ship:AddEquip(laserdef)
			end
			mission.ship = ship
		end

		if mission.status == "ACTIVE" and Game.time > mission.due then
			mission.status = "FAILED"
		end
	end
end

local onShipDocked = function (player, station)
	if not player:IsPlayer() then return end

	for ref, mission in pairs(missions) do
		local msg

		if mission.interceptor then
			if station.type == "STARPORT_SURFACE" then
				mission.interceptor:AIEnterLowOrbit(Space.GetBody(station:GetSystemBody().parent.index))
			else
				mission.interceptor:AIFlyTo(station)
			end
		end

		if mission.halfdone then
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
					player:AddMoney(mission.reward)
				else
					msg = string.interp(l["FAILUREMSG_" .. Engine.rand:Integer(1, getNumberOfFlavours("FAILUREMSG"))], { wanted = mission.wanted.name })
					Comms.ImportantMessage(msg, mission.client.name)
					Character.persistent.player.reputation = Character.persistent.player.reputation - reputation
				end
				if mission.flavour.taxi then
					player:RemoveEquip(Equipment.misc.cabin_occupied)
					player:AddEquip(Equipment.misc.cabin)
				end
				Event.Queue("onReputationChanged", oldReputation, Character.persistent.player.killcount,
					Character.persistent.player.reputation, Character.persistent.player.killcount)
				mission:Remove()
				missions[ref] = nil
			end
		else
			if mission.destination == station.path then
				msg = string.interp(l["GREETING_" .. mission.flavour.id], { client = mission.client.name })
				Comms.ImportantMessage(msg, mission.wanted.name)
				if mission.flavour.taxi then
					if player:CountEquip(Equipment.misc.cabin) > 0 then
						player:RemoveEquip(Equipment.misc.cabin)
						player:AddEquip(Equipment.misc.cabin_occupied)
						mission.halfdone = true
					else
						-- cabin occupied or player has removed cabin?
						Comms.ImportantMessage(l.YOU_DO_NOT_HAVE_A_CABIN, mission.wanted.name)
					end
				else
					mission.halfdone = true
				end
			else
				-- do nothing if not in the right system or a tipster was already there
				if mission.destination:IsSameSystem(Game.system.path) and not mission.tipster then
					-- don't increase probability if last station is current station
					if station.path ~= mission.visited[#mission.visited] then
						table.insert(mission.visited, station.path)
					end
					if #mission.visited > Engine.rand:Number(4) then
						local tipster = Character.New()
						local tip = "TIP_" .. (mission.wanted.female and "FEMALE" or "MALE")
						msg = string.interp(l[tip .. "_" .. Engine.rand:Integer(1, getNumberOfFlavours(tip))], { wanted = mission.wanted.name, station = mission.destination:GetSystemBody().name })
						Comms.ImportantMessage(msg, tipster.name)
						mission.tipster = true
					end
				end
			end
		end
	end
end

local onShipUndocked = function (player, station)
	if not player:IsPlayer() then return end

	for ref, mission in pairs(missions) do
		if mission.interceptor then
			mission.interceptor:AIKill(player)
		end
	end
end

local onLeaveSystem = function (ship)
	if ship:IsPlayer() then
		nearbysystems = nil
		for ref, mission in pairs(missions) do
			mission.ship = nil
			mission.interceptor = nil
		end
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
	if loaded_data then
		ads = {}
		missions = {}

		for k, ad in pairs(loaded_data.ads) do
			local ref = ad.station:AddAdvert({
				description = ad.desc,
				icon        = "default",
				onChat      = onChat,
				onDelete    = onDelete,
				isEnabled   = isEnabled })
			ads[ref] = ad
		end
		missions = loaded_data.missions
		loaded_data = nil
	end
end

local onGameEnd = function ()
	nearbysystems = nil
end

local onClick = function (mission)
	local dist = Game.system and string.format("%.2f", Game.system:DistanceTo(mission.location)) or "???"
	local gender = (mission.wanted.female and "_FEMALE" or "_MALE")
	local danger = l:get("RISK_" .. mission.flavour.id .. gender .. "_" .. math.ceil(mission.risk * getNumberOfFlavours("RISK_" .. mission.flavour.id .. gender)))
		or l["RISK" .. gender .. "_" .. math.ceil(mission.risk * getNumberOfFlavours("RISK" .. gender))]
	return ui:Grid(2,1)
		:SetColumn(0, {
			ui:VBox():PackEnd({
				ui:MultiLineText(
					mission.introtext:interp({
						client = mission.client.name,
						wanted = mission.wanted.name,
						company = mission.company,
						system = mission.destination:GetStarSystem().name,
						sectorx = mission.destination.sectorX,
						sectory = mission.destination.sectorY,
						sectorz = mission.destination.sectorZ,
						shipid = mission.shipid,
						domicile = mission.domicile:GetSystemBody().name,
						cash = Format.Money(mission.reward, false),
						dist = dist
					})
				),
				ui:Margin(10),
				ui:Grid(2,1)
					:SetColumn(0, {
						ui:VBox():PackEnd({
							ui:Label(l.WANTED)
						})
					})
					:SetColumn(1, {
						ui:VBox():PackEnd({
							ui:Label(mission.wanted.name)
						})
					}),
				ui:Grid(2,1)
					:SetColumn(0, {
						ui:VBox():PackEnd({
							ui:Label(l.SYSTEM)
						})
					})
					:SetColumn(1, {
						ui:VBox():PackEnd({
							ui:MultiLineText(mission.location:GetStarSystem().name.." ("..mission.location.sectorX..","..mission.location.sectorY..","..mission.location.sectorZ..")")
						})
					}),
				mission.flavour.ship and ui:Grid(2,1)
					:SetColumn(0, {
						ui:VBox():PackEnd({
							ui:Label(l.SHIP)
						})
					})
					:SetColumn(1, {
						ui:VBox():PackEnd({
							ui:Label(mission.shipid)
						})
					}) or ui:Margin(0),
				ui:Grid(2,1)
					:SetColumn(0, {
						ui:VBox():PackEnd({
							ui:Label(l.DISTANCE)
						})
					})
					:SetColumn(1, {
						ui:VBox():PackEnd({
							ui:Label(dist .. " " .. l.LY)
						})
					}),
				NavButton.New(l.SET_AS_TARGET, mission.location),
				ui:Margin(5),
				ui:Grid(2,1)
					:SetColumn(0, {
						ui:VBox():PackEnd({
							ui:Label(l.CLIENT)
						})
					})
					:SetColumn(1, {
						ui:VBox():PackEnd({
							ui:Label(mission.client.name)
						})
					}),
				mission.company and ui:Grid(2,1)
					:SetColumn(0, {
						ui:VBox():PackEnd({
							ui:Label(l.COMPANY)
						})
					})
					:SetColumn(1, {
						ui:VBox():PackEnd({
							ui:Label(mission.company)
						})
					}) or ui:Margin(0),
				ui:Grid(2,1)
					:SetColumn(0, {
						ui:VBox():PackEnd({
							ui:Label(l.SYSTEM)
						})
					})
					:SetColumn(1, {
						ui:VBox():PackEnd({
							ui:MultiLineText(mission.domicile:GetStarSystem().name.." ("..mission.domicile.sectorX..","..mission.domicile.sectorY..","..mission.domicile.sectorZ..")")
						})
					}),
				ui:Grid(2,1)
					:SetColumn(0, {
						ui:VBox():PackEnd({
							ui:Label(l.SPACEPORT)
						})
					})
					:SetColumn(1, {
						ui:VBox():PackEnd({
							ui:Label(mission.domicile:GetSystemBody().name)
						})
					}),
				ui:Grid(2,1)
					:SetColumn(0, {
						ui:VBox():PackEnd({
							ui:Label(l.DISTANCE)
						})
					})
					:SetColumn(1, {
						ui:VBox():PackEnd({
							ui:Label((Game.system and string.format("%.2f", Game.system:DistanceTo(mission.domicile)) or "???") .. " " .. l.LY)
						})
					}),
				NavButton.New(l.SET_RETURN_ROUTE, mission.domicile),
				ui:Margin(5),
				ui:Grid(2,1)
					:SetColumn(0, {
						ui:VBox():PackEnd({
							ui:Label(l.DEADLINE)
						})
					})
					:SetColumn(1, {
						ui:VBox():PackEnd({
							ui:Label(Format.Date(mission.due))
						})
					}),
				ui:Grid(2,1)
					:SetColumn(0, {
						ui:VBox():PackEnd({
							ui:Label(l.PROGRESS)
						})
					})
					:SetColumn(1, {
						ui:VBox():PackEnd({
							ui:Label(mission.halfdone and l.MSTAT_HALF_DONE or l.MSTAT_NONE)
						})
					}),
				ui:Grid(2,1)
					:SetColumn(0, {
						ui:VBox():PackEnd({
							ui:Label(l.DANGER)
						})
					})
					:SetColumn(1, {
						ui:VBox():PackEnd({
							ui:MultiLineText(string.interp(danger, { wanted = mission.wanted.name }))
						})
					}),
			})
		})
		:SetColumn(1, {
			ui:VBox(10):PackEnd(InfoFace.New(mission.client))
		})
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
Event.Register("onShipDocked", onShipDocked)
Event.Register("onShipUndocked", onShipUndocked)
Event.Register("onShipFiring", onShipFiring)
Event.Register("onShipHit", onShipHit)
Event.Register("onShipDestroyed", onShipDestroyed)
Event.Register("onGameStart", onGameStart)
Event.Register("onGameEnd", onGameEnd)
Event.Register("onReputationChanged", onReputationChanged)

Mission.RegisterType("FindPerson", l.FIND_PERSON, onClick)

Serializer:Register("FindPerson", serialize, unserialize)

