-- Copyright © 2008-2024 Pioneer Developers. See AUTHORS.txt for details
-- Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

local Engine = require 'Engine'
local Lang = require 'Lang'
local Game = require 'Game'
local Space = require 'Space'
local Comms = require 'Comms'
local Event = require 'Event'
local Timer = require 'Timer'
local Mission = require 'Mission'
local MissionUtils = require 'modules.MissionUtils'
local Format = require 'Format'
local Serializer = require 'Serializer'
local Character = require 'Character'
local NameGen = require 'NameGen'
local Equipment = require 'Equipment'
local ShipDef = require 'ShipDef'
local Ship = require 'Ship'
local utils = require 'utils'

local l = Lang.GetResource("module-combat")
local lc = Lang.GetResource 'core'

-- typical reward for a mission to a system 1ly away
local typical_reward = 100

-- Mission subtypes
local RECON = 1/3
local ARMEDRECON = 2/3
local AREASWEEP = 1
local NUMSUBTYPES = 3

local flavours = {
	{
		id = "MINING", max_dist = 20,
		planet_type = "PLANET_ASTEROID", planets = nil, needs_faction = false,
		client_title = "MANAGER", is_multi = true,
		enemy = "merchant"
	},
	{
		id = "POLICE", max_dist = 10,
		planet_type = "PLANET_TERRESTRIAL", planets = nil, needs_faction = true,
		client_title = "DETECTIVE", is_multi = false,
		enemy = "pirate"
	},
	{
		id = "MILITARY", max_dist = 15,
		planet_type = "PLANET_TERRESTRIAL", planets = nil, needs_faction = false,
		client_title = "AGENT", is_multi = false,
		enemy = "mercenary"
	},
	{
		id = "HARVESTING", max_dist = 20,
		planet_type = "PLANET_GAS_GIANT", planets = nil, needs_faction = false,
		client_title = "MANAGER", is_multi = true,
		enemy = "merchant"
	},
}

local ads = {}
local missions = {}

local isQualifiedFor = function(reputation, killcount, ad)
	return reputation >= 16 and (
		killcount >= 8 or
		killcount >= 4 and ad.dedication <= ARMEDRECON or
		ad.dedication <= RECON or
		false
	)
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

	local qualified = isQualifiedFor(Character.persistent.player.reputation, Character.persistent.player.killcount, ad)

	form:SetFace(ad.client)

	if not qualified then
		form:SetMessage(l["DENY_" .. Engine.rand:Integer(1, getNumberOfFlavours("DENY"))])
		return
	end

	form:AddNavButton(ad.location)

	if option == 0 then
		local introtext = string.interp(ad.introtext, {
			name    = ad.client.name,
			org     = ad.org,
			cash    = Format.Money(ad.reward, false),
			area    = ad.location:GetSystemBody().name,
			system  = ad.location:GetStarSystem().name,
			sectorx = ad.location.sectorX,
			sectory = ad.location.sectorY,
			sectorz = ad.location.sectorZ,
			mission = l["MISSION_TYPE_" .. math.ceil(ad.dedication * NUMSUBTYPES)],
			dist    = string.format("%.2f", ad.dist),
		})
		form:SetMessage(introtext)

	elseif option == 1 then
		form:SetMessage(string.interp(l["OBJECTIVE_" .. math.ceil(ad.dedication * NUMSUBTYPES)], { area = ad.location:GetSystemBody().name }))

	elseif option == 2 then
		form:SetMessage(l["RISK_" .. math.ceil(ad.risk * (getNumberOfFlavours("RISK")))])

	elseif option == 3 then
		form:SetMessage(string.interp(l[ad.flavour.id .. "_IT_MUST_BE_COMPLETED_BY"], { area = ad.location:GetSystemBody().name, date = Format.Date(ad.due) }))

	elseif option == 4 then
		if ad.rendezvous then
			form:SetMessage(string.interp(l.MEET_ME_THERE, { rendezvous = ad.rendezvous:GetStarSystem().name }))
		else
			form:SetMessage(string.interp(l[ad.flavour.id .. "_LAND_THERE"], { org = ad.org }))
		end

	elseif option == 5 then
		if not Game.player:GetEquip('radar', 1) then
			form:SetMessage(l.RADAR_NOT_INSTALLED)
			form:RemoveNavButton()
			return
		end
		form:RemoveAdvertOnClose()
		ads[ref] = nil
		local mission = {
			type        = "Combat",
			client      = ad.client,
			faction     = Game.system.faction.id,
			org         = ad.org,
			location    = ad.location,
			rendezvous  = ad.rendezvous,
			mercenaries = {},
			introtext   = ad.introtext,
			flavour     = ad.flavour,
			in_progress = false,
			complete    = false,
			risk        = ad.risk,
			dedication  = ad.dedication,
			reward      = ad.reward,
			bonus       = -(math.ceil(ad.dedication * 3) - 1),
			due         = ad.due,
		}
		table.insert(missions,Mission.New(mission))
		form:SetMessage(l["ACCEPTED_" .. Engine.rand:Integer(1, getNumberOfFlavours("ACCEPTED"))])
		return
	elseif option == 6 then
		form:SetMessage(l.YOU_NEED_A_RADAR)
	end

	form:AddOption(l.WHAT_ARE_THE_MISSION_OBJECTIVES, 1)
	form:AddOption(l.WILL_I_BE_IN_TROUBLE, 2)
	form:AddOption(l.IS_THERE_A_TIME_LIMIT, 3)
	form:AddOption(l.HOW_WILL_I_BE_PAID, 4)
	form:AddOption(l.DO_I_NEED_SPECIAL_EQUIPMENT, 6)
	form:AddOption(l.PLEASE_REPEAT_THE_MISSION_DETAILS, 0)
	form:AddOption(l.OK_AGREED, 5)
end

local onDelete = function (ref)
	ads[ref] = nil
end

local isEnabled = function (ref)
	return ads[ref] ~= nil and isQualifiedFor(Character.persistent.player.reputation, Character.persistent.player.killcount, ads[ref])
end

local findPlanets = function (dist, type, fac)
	local planets = {}
	local nearbysystems = Game.system:GetNearbySystems(dist,
		function (s)
			return #s:GetBodyPaths() > 0 and (s.lawlessness > 0.75 or s.population == 0) and (fac == nil or fac == s.faction.id)
		end)
	for _, s in pairs(nearbysystems) do
		for _, p in ipairs(s:GetBodyPaths()) do
			if p:GetSystemBody().type == type then
				table.insert(planets, p)
			end
		end
	end
	return planets
end

local placeAdvert = function (station, ad)
	local title = l["ADTITLE_" .. ad.titleId]
	local desc = string.interp(l["ADTEXT_" .. ad.titleId], {
		system = ad.location:GetStarSystem().name,
		cash = Format.Money(ad.reward, false),
		mission = l["MISSION_TYPE_" .. math.ceil(ad.dedication * NUMSUBTYPES)],
		org = ad.org
	})

	local ref = station:AddAdvert({
		title       = title,
		description = desc,
		icon        = "combat",
		due         = ad.due,
		reward      = ad.reward,
		location    = ad.location,
		onChat      = onChat,
		onDelete    = onDelete,
		isEnabled   = isEnabled})
	ads[ref] = ad
end

local makeAdvert = function (station)
	local flavour, location, dist, reward, due, org, time, timeout
	local risk = Engine.rand:Number(0.2, 1)
	local dedication = Engine.rand:Number(0.1, 1)
	local urgency = Engine.rand:Number(1)
	local rendezvous = nil

	flavour = flavours[Engine.rand:Integer(1, #flavours)]
	if flavour.planets == nil then
		flavour.planets = findPlanets(flavour.max_dist, flavour.planet_type, flavour.needs_faction and Game.system.faction.id or nil)
	end
	if #flavour.planets == 0 then return end

	org = flavour.id == "POLICE" and Game.system.faction.policeName or
		flavour.id == "MILITARY" and Game.system.faction.militaryName or
		string.interp(l["CORPORATION_" .. Engine.rand:Integer(1, getNumberOfFlavours("CORPORATION"))], { name = NameGen.Surname() } )

	location = flavour.planets[Engine.rand:Integer(1, #flavour.planets)]
	dist = location:DistanceTo(Game.system)
	reward = math.ceil(dist * typical_reward * (1 + dedication)^2 * (1 + risk) * (1 + urgency) * Engine.rand:Number(0.8, 1.2))
	reward = utils.round(reward, 100)
	time = Engine.rand:Number(21*24*60*60, 28*24*60*60)
	due = time + MissionUtils.TravelTime(dist, location) * 2 * (1.5 - urgency)
	timeout = due/2 + Game.time -- timeout after half of the travel time
	due = utils.round(due + Game.time, 3600)

	if Engine.rand:Number(1) > 0.5 then
		local nearbysystems = location:GetStarSystem():GetNearbySystems(10)
		if #nearbysystems ~= 0 then
			rendezvous = nearbysystems[Engine.rand:Integer(1, #nearbysystems)]
		end
	end

	local titleNum = Engine.rand:Integer(1, getNumberOfFlavours("ADTEXT"))
	local introtext = l["GREETING_" .. Engine.rand:Integer(1, getNumberOfFlavours("GREETING"))] .. " " .. l[flavour.id .. "_" .. Engine.rand:Integer(1, getNumberOfFlavours(flavour.id))]

	local ad = {
		station     = station,
		titleId     = titleNum,
		introtext   = introtext,
		flavour     = flavour,
		client      = Character.New( { title = l[flavour.client_title] } ),
		org         = org,
		location    = location,
		rendezvous  = rendezvous and rendezvous.path,
		dist        = dist,
		risk        = risk,
		dedication  = dedication,
		urgency     = urgency,
		reward      = reward,
		due         = due,
		timeout     = timeout,
	}

	placeAdvert(station, ad)
end

local onCreateBB = function (station)
	local num = Engine.rand:Integer(0, math.ceil(Game.system.population * 2 * Game.system.lawlessness) + 1)
	for i = 1, num do
		makeAdvert(station)
	end
end

local onUpdateBB = function (station)
	for ref, ad in pairs(ads) do
		if ad.timeout < Game.time then
			ad.station:RemoveAdvert(ref)
		end
	end
	if Engine.rand:Integer(4*24*60*60) < 60*60 then -- roughly once every four days
		makeAdvert(station)
	end
end

local onShipDestroyed = function (ship, attacker)
	if ship:IsPlayer() then return end

	for ref, mission in pairs(missions) do
		for i, s in pairs(mission.mercenaries) do
			if s == ship then
				table.remove(mission.mercenaries, i)
				if not mission.complete and (#mission.mercenaries == 0 or mission.dedication <= ARMEDRECON) then
					mission.complete = true
					mission.status = "COMPLETED"
					Comms.ImportantMessage(l.MISSION_COMPLETE)
				end
				if attacker and attacker:isa("Ship") and attacker:IsPlayer() then
					mission.bonus = mission.bonus + 1
				end
				break
			end
		end
	end
end

local missionTimer = function (mission)
	Timer:CallEvery(mission.duration, function ()
		if mission.complete or Game.time > mission.due then return true end -- already complete or too late
		if Game.player.frameBody and Game.player.frameBody.path == mission.location then
			mission.complete = true
			mission.status = "COMPLETED"
			Comms.ImportantMessage(l.MISSION_COMPLETE)
			return true
		else
			Comms.ImportantMessage(l.MISSION_WARNING)
		end
	end)
end

local onFrameChanged = function (player)
	if not player:isa("Ship") or not player:IsPlayer() then return end

	for ref, mission in pairs(missions) do
		if player.frameBody and player.frameBody.path == mission.location and not mission.in_progress then
			mission.in_progress = true

			local ships
			local planet_radius = player.frameBody:GetPhysicalRadius()
			local riskmargin = Engine.rand:Number(-0.3, 0.3) -- Add some random luck
			if mission.risk >= (1 + riskmargin) then ships = 3
			elseif mission.risk >= (0.7 + riskmargin) then ships = 2
			elseif mission.risk >= (0.5 + riskmargin) then ships = 1
			else ships = 0
			end

			if ships < 1 and Engine.rand:Integer(math.ceil(1/mission.risk)) == 1 then ships = 1 end

			local shipdefs = utils.build_array(utils.filter(
				function (k,def)
					return def.tag == "SHIP" and def.hyperdriveClass > 0 and def.equipSlotCapacity.laser_front > 0 and def.roles[mission.flavour.enemy]
				end,
				pairs(ShipDef)))
			if #shipdefs == 0 then ships = 0 end

			while ships > 0 do
				ships = ships - 1

				if Engine.rand:Number(1) <= mission.risk then
					local shipdef = shipdefs[Engine.rand:Integer(1, #shipdefs)]
					local default_drive = Equipment.hyperspace["hyperdrive_" .. tostring(shipdef.hyperdriveClass)]

					local max_laser_size = shipdef.capacity - default_drive.capabilities.mass
					local laserdefs = utils.build_array(utils.filter(
						function (k,l)
							return l:IsValidSlot("laser_front") and l.capabilities.mass <= max_laser_size and l.l10n_key:find("PULSECANNON")
						end,
						pairs(Equipment.laser)
					))
					local laserdef = laserdefs[Engine.rand:Integer(1, #laserdefs)]

					local ship
					if mission.location:GetSystemBody().type == "PLANET_GAS_GIANT" then
						ship = Space.SpawnShipOrbit(shipdef.id, player.frameBody, 1.2 * planet_radius, 3.5 * planet_radius)
					else
						ship = Space.SpawnShipLanded(shipdef.id, player.frameBody, math.rad(Engine.rand:Number(360)), math.rad(Engine.rand:Number(360)))
					end
					ship:SetLabel(Ship.MakeRandomLabel())
					ship:AddEquip(default_drive)
					ship:AddEquip(laserdef)
					ship:AddEquip(Equipment.misc.shield_generator, math.ceil(mission.risk * 3))
					if Engine.rand:Number(2) <= mission.risk then
						ship:AddEquip(Equipment.misc.laser_cooling_booster)
					end
					if Engine.rand:Number(3) <= mission.risk then
						ship:AddEquip(Equipment.misc.shield_energy_booster)
					end
					table.insert(mission.mercenaries, ship)
					ship:AIEnterLowOrbit(Space.GetBody(mission.location.bodyIndex))
				end
			end

			if #mission.mercenaries ~= 0 then
				Timer:CallAt(Game.time + 5, function ()
					for _, s in pairs(mission.mercenaries) do
						s:AIKill(player)
					end
				end)
			end

			if mission.dedication <= RECON or #mission.mercenaries == 0 then
				-- prevent a too quick fly-by
				mission.duration = planet_radius/1000
				Comms.ImportantMessage(string.interp(l.MISSION_INFO, { duration = Format.Duration(mission.duration) }))
				missionTimer(mission)
			else
				Comms.ImportantMessage(l.TARGET_AREA_REACHED)
			end
		end
		if mission.status == "ACTIVE" and Game.time > mission.due then
			mission.status = "FAILED"
		end
	end
end

local finishMission = function (ref, mission)
	local delta_reputation = 0
	if Game.time > mission.due then
		Comms.ImportantMessage(l["FAILUREMSG_" .. Engine.rand:Integer(1, getNumberOfFlavours("FAILUREMSG"))], mission.client.name)
		delta_reputation = -2.5
	elseif mission.complete then
		Comms.ImportantMessage(l["SUCCESSMSG_" .. Engine.rand:Integer(1, getNumberOfFlavours("SUCCESSMSG"))], mission.client.name)
		delta_reputation = 2.5
		Game.player:AddMoney(mission.reward)
		if mission.bonus > 0 then
			local bonus = math.ceil(mission.reward/5 * mission.bonus)
			local addition = string.interp(l["BONUS_" .. Engine.rand:Integer(1, getNumberOfFlavours("BONUS"))], { cash = Format.Money(bonus, false) })
			Comms.ImportantMessage(addition, mission.client.name)
			Game.player:AddMoney(bonus)
		end
	end
	if delta_reputation ~= 0 then
		local oldReputation = Character.persistent.player.reputation
		Character.persistent.player.reputation = Character.persistent.player.reputation + delta_reputation
		Event.Queue("onReputationChanged", oldReputation, Character.persistent.player.killcount,
			Character.persistent.player.reputation, Character.persistent.player.killcount)
		mission:Remove()
		missions[ref] = nil
	end
end

local onEnterSystem = function (player)
	if not player:IsPlayer() then return end

	for ref, mission in pairs(missions) do
		if mission.rendezvous and mission.rendezvous:IsSameSystem(Game.system.path) then
			if mission.complete or Game.time > mission.due then
				local shipdefs = utils.build_array(utils.filter(
					function (k,def)
						return def.tag == "SHIP" and def.hyperdriveClass > 0
					end,
					pairs(ShipDef)))
				local shipdef = shipdefs[Engine.rand:Integer(1, #shipdefs)]

				local ship = Space.SpawnShipNear(shipdef.id, Game.player, 50, 100)
				ship:SetLabel(Ship.MakeRandomLabel())
				ship:AddEquip(Equipment.hyperspace["hyperdrive_" .. tostring(shipdef.hyperdriveClass)])

				local path = mission.location:GetStarSystem().path
				finishMission(ref, mission)
				ship:HyperjumpTo(path)
			end
		end
	end
end

local onLeaveSystem = function (ship)
	if ship:IsPlayer() then
		for _, f in pairs(flavours) do
			f.planets = nil
		end
		for ref, mission in pairs(missions) do
			mission.mercenaries = {}
		end
	end
end

local onShipDocked = function (player, station)
	if not player:IsPlayer() then return end

	for ref, mission in pairs(missions) do
		if not mission.rendezvous and (mission.flavour.is_multi or mission.faction == Game.system.faction.id) then
			finishMission(ref, mission)
		end
	end
end

local onReputationChanged = function (oldRep, oldKills, newRep, newKills)
	for ref, ad in pairs(ads) do
		local oldQualified = isQualifiedFor(oldRep, oldKills, ad)
		if isQualifiedFor(newRep, newKills, ad) ~= oldQualified then
			Event.Queue("onAdvertChanged", ad.station, ref);
		end
	end
end

local loaded_data

local onGameStart = function ()
	ads = {}
	missions = {}

	if loaded_data and loaded_data.ads then
		for k, ad in pairs(loaded_data.ads) do
			placeAdvert(ad.station, ad)
		end
		missions = loaded_data.missions
		loaded_data = nil

		for _, mission in pairs(missions) do
			if mission.duration then
				missionTimer(mission)
			end
		end
	end
end

local onGameEnd = function ()
	for _, f in pairs(flavours) do
		f.planets = nil
	end
end

local buildMissionDescription = function(mission)
	local ui = require 'pigui'

	local desc = {}
	local dist = Game.system and string.format("%.2f", Game.system:DistanceTo(mission.location)) or "???"
	local type = l["MISSION_TYPE_" .. math.ceil(mission.dedication * NUMSUBTYPES)]

	desc.description = mission.introtext:interp({
		name    = mission.client.name,
		org     = mission.org,
		cash    = Format.Money(mission.reward, false),
		area    = mission.location:GetSystemBody().name,
		system  = mission.location:GetStarSystem().name,
		sectorx = mission.location.sectorX,
		sectory = mission.location.sectorY,
		sectorz = mission.location.sectorZ,
		mission = type,
		dist    = dist
	})

	desc.client = mission.client
	desc.location = mission.location

	local paymentLoc = mission.rendezvous and ui.Format.SystemPath(mission.rendezvous)
		or string.interp(l[mission.flavour.id .. "_LAND_THERE"], { org = mission.org })

	desc.details = {
		{ l.MISSION, type },
		{ l.SYSTEM, ui.Format.SystemPath(mission.location) },
		{ l.AREA, mission.location:GetSystemBody().name },
		{ l.DISTANCE, dist.." "..lc.UNIT_LY },
		{ l.TIME_LIMIT, ui.Format.Date(mission.due) },
		{ l.DANGER, l["RISK_" .. math.ceil(mission.risk * (getNumberOfFlavours("RISK")))] },
		{ l.PAYMENT_LOCATION, paymentLoc }
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
Event.Register("onEnterSystem", onEnterSystem)
Event.Register("onFrameChanged", onFrameChanged)
Event.Register("onLeaveSystem", onLeaveSystem)
Event.Register("onShipDocked", onShipDocked)
Event.Register("onShipDestroyed", onShipDestroyed)
Event.Register("onGameStart", onGameStart)
Event.Register("onGameEnd", onGameEnd)
Event.Register("onReputationChanged", onReputationChanged)

Mission.RegisterType("Combat",l.COMBAT, buildMissionDescription)

Serializer:Register("Combat", serialize, unserialize)
