-- Copyright © 2008-2024 Pioneer Developers. See AUTHORS.txt for details
-- Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

local Engine = require 'Engine'
local Lang = require 'Lang'
local Game = require 'Game'
local Space = require 'Space'
local Comms = require 'Comms'
local Timer = require 'Timer'
local Event = require 'Event'
local Mission = require 'Mission'
local MissionUtils = require 'modules.MissionUtils'
local NameGen = require 'NameGen'
local Character = require 'Character'
local Commodities = require 'Commodities'
local Format = require 'Format'
local Serializer = require 'Serializer'
local Equipment = require 'Equipment'
local ShipDef = require 'ShipDef'
local Ship = require 'Ship'
local utils = require 'utils'

local lc = Lang.GetResource 'core'
local l = Lang.GetResource("module-assassination")

-- don't produce missions for further than this many light years away
local max_ass_dist = 30

local flavours = {}
for i = 0,5 do
	table.insert(flavours, {
		adtitle     = l["FLAVOUR_" .. i .. "_ADTITLE"],
		adtext      = l["FLAVOUR_" .. i .. "_ADTEXT"],
		introtext   = l["FLAVOUR_" .. i .. "_INTROTEXT"],
		successmsg  = l["FLAVOUR_" .. i .. "_SUCCESSMSG"],
		failuremsg  = l["FLAVOUR_" .. i .. "_FAILUREMSG"],
		failuremsg2 = l["FLAVOUR_" .. i .. "_FAILUREMSG2"],
	})
end
local num_titles = 25
local num_deny = 8

local ads = {}
local missions = {}

local isQualifiedFor = function(reputation, kills, ad)
	return reputation >= 16 and
		(kills >= 16 or
		 kills >=  4 and ad.danger <= 1 or
		 kills >=  8 and ad.danger <  4 or
		 false)
end

local onDelete = function (ref)
	ads[ref] = nil
end

local isEnabled = function (ref)
	return ads[ref] ~= nil and isQualifiedFor(Character.persistent.player.reputation, Character.persistent.player.killcount, ads[ref])
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
		local introtext = l["DENY_"..Engine.rand:Integer(1,num_deny)-1]
		form:SetMessage(introtext)
		return
	end

	form:AddNavButton(ad.location)

	if option == 0 then
		local sys = ad.location:GetStarSystem()

		local introtext = string.interp(flavours[ad.flavour].introtext, {
			name	= ad.client.name,
			cash	= Format.Money(ad.reward,false),
			target	= ad.target,
			system	= sys.name,
		})
		form:SetMessage(introtext)

	elseif option == 1 then
		local sys = ad.location:GetStarSystem()
		local sbody = ad.location:GetSystemBody()

		form:SetMessage(string.interp(l.X_WILL_BE_LEAVING, {
		  target    = ad.target,
		  spaceport = sbody.name,
		  system    = sys.name,
		  sectorX   = ad.location.sectorX,
		  sectorY   = ad.location.sectorY,
		  sectorZ   = ad.location.sectorZ,
		  dist      = string.format("%.2f", ad.dist),
		  date      = Format.Date(ad.due),
		  shipname  = ad.shipname,
		  shipregid = ad.shipregid,
		  })
		)

	elseif option == 2 then
		local sbody = ad.location:GetSystemBody()

		form:SetMessage(string.interp(l.IT_MUST_BE_DONE_AFTER, {
		  target    = ad.target,
		  spaceport = sbody.name,
		}))

	elseif option == 3 then
		local backstation = Game.player:GetDockedWith().path

		form:RemoveAdvertOnClose()

		ads[ref] = nil

		local mission = {
			type		= "Assassination",
			backstation	= backstation,
			client		= ad.client,
			danger		= ad.danger,
			due		= ad.due,
			flavour		= ad.flavour,
			location	= ad.location,
			reward		= ad.reward,
			shipid		= ad.shipid,
			shipname	= ad.shipname,
			shipregid	= ad.shipregid,
			status		= 'ACTIVE',
			target		= ad.target,
		}

		table.insert(missions,Mission.New(mission))

		form:SetMessage(l.EXCELLENT)

		return
	elseif option == 4 then
		form:SetMessage(l.RETURN_HERE_ON_THE_COMPLETION_OF_THE_CONTRACT_AND_YOU_WILL_BE_PAID)
	end
	form:AddOption(string.interp(l.WHERE_CAN_I_FIND_X, {target = ad.target}), 1);
	form:AddOption(l.COULD_YOU_REPEAT_THE_ORIGINAL_REQUEST, 0);
	form:AddOption(l.HOW_SOON_MUST_IT_BE_DONE, 2);
	form:AddOption(l.HOW_WILL_I_BE_PAID, 4);
	form:AddOption(l.OK_AGREED, 3);
end

local placeAdvert = function(station, ad)
	local desc = string.interp(flavours[ad.flavour].adtext, {
		target	= ad.target,
		system	= ad.location:GetStarSystem().name,
	})

	local ref = station:AddAdvert({
		title       = flavours[ad.flavour].adtitle,
		description = desc,
		icon        = "assassination",
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
		nearbysystems = Game.system:GetNearbySystems(max_ass_dist, function (s) return #s:GetStationPaths() > 0 end)
	end
	if #nearbysystems == 0 then return end
	local client = Character.New()
	local targetIsfemale = Engine.rand:Integer(1) == 1
	local target = l["TITLE_"..Engine.rand:Integer(1, num_titles)-1] .. " " .. NameGen.FullName(targetIsfemale)
	local flavour = Engine.rand:Integer(1, #flavours)
	local nearbysystem = nearbysystems[Engine.rand:Integer(1,#nearbysystems)]
	local nearbystations = nearbysystem:GetStationPaths()
	local location = nearbystations[Engine.rand:Integer(1,#nearbystations)]
	local dist = location:DistanceTo(Game.system)
	local time = Engine.rand:Number(7*60*60*24, 35*60*60*24)
	local due = time + MissionUtils.TravelTime(dist, location) * Engine.rand:Number(0.5, 1.5)
	local timeout = Game.time + due/2
	local danger = Engine.rand:Integer(1,4)
	local reward = Engine.rand:Number(2100, 7000) * danger
	reward = utils.round(reward, 500)
	due = utils.round(due + Game.time, 3600)

	-- XXX hull mass is a bad way to determine suitability for role
	--local shipdefs = utils.build_array(utils.filter(function (k,def) return def.tag == 'SHIP' and def.hullMass >= (danger * 17) and def.equipSlotCapacity.ATMOSHIELD > 0 end, pairs(ShipDef)))
	local shipdefs = utils.build_array(utils.filter(function (k,def) return def.tag == 'SHIP' and def.hyperdriveClass > 0 and def.equipSlotCapacity.atmo_shield > 0 end, pairs(ShipDef)))
	local shipdef = shipdefs[Engine.rand:Integer(1,#shipdefs)]
	local shipid = shipdef.id
	local shipname = shipdef.name

	local ad = {
		client = client,
		danger = danger,
		due = due,
		faceseed = Engine.rand:Integer(),
		flavour = flavour,
		location = location,
		dist = dist,
		reward = reward,
		shipid = shipid,
		shipname = shipname,
		shipregid = Ship.MakeRandomLabel(),
		station = station,
		target = target,
		timeout = timeout,
	}

	placeAdvert(station, ad)
end

local onCreateBB = function (station)
	local num = Engine.rand:Integer(0, math.ceil(Game.system.population) / 2 + 1)
	for i = 1,num do
		makeAdvert(station)
	end
end

local onShipHit = function (ship, attacker)
	if attacker and not attacker:IsPlayer() then return end -- XX

	-- When the player attacks the target, make it fight back
	for k,mission in pairs(missions) do
		if mission.status == 'ACTIVE' and
		   mission.ship == ship then
			ship:AIKill(attacker)
			return
		end
	end
end

local onShipDestroyed = function (ship, body)
	for ref, mission in pairs(missions) do
		if mission.status == 'ACTIVE' and
		   mission.ship == ship and
		   mission.due < Game.time then
			if not body:isa("Ship") or
			   not body:IsPlayer() then
				mission.status = 'FAILED'
				mission.notplayer = 'TRUE'
			else -- well done, comrade
				mission.status = 'COMPLETED'
				mission.location = mission.backstation
				mission.notplayer = 'FALSE'
			end
			mission.ship = nil
			return
		end
	end
end

local _setupHooksForMission = function (mission)
	if mission.ship:exists() and
	   mission.due > Game.time then
		-- Target hasn't launched yet. set up a timer to do this
		Timer:CallAt(mission.due, function () if mission.ship:exists() then mission.ship:Undock()
			mission.timer = nil end end)
		mission.timer = 'SET'
	end
end

local planets
local onEnterSystem = function (ship)
	if not ship:IsPlayer() then return end

	local syspath = Game.system.path

	for ref,mission in pairs(missions) do
		if mission.status == 'ACTIVE' then
			if not mission.ship then
				if mission.due > Game.time then
					if mission.location:IsSameSystem(syspath) then -- spawn our target ship
						local station = Space.GetBody(mission.location.bodyIndex)
						local shiptype = ShipDef[mission.shipid]
						local default_drive = shiptype.hyperdriveClass
						local laserdefs = utils.build_array(pairs(Equipment.laser))
						table.sort(laserdefs, function (l1, l2) return l1.price < l2.price end)
						local laserdef = laserdefs[mission.danger]
						local count = default_drive ^ 2

						mission.ship = Space.SpawnShipDocked(mission.shipid, station)
						if mission.ship == nil then
							return -- TODO
						end

						mission.ship:SetLabel(mission.shipregid)

						mission.ship:AddEquip(Equipment.misc.atmospheric_shielding)
						local engine = Equipment.hyperspace['hyperdrive_'..tostring(default_drive)]
						mission.ship:AddEquip(engine)
						mission.ship:AddEquip(laserdef)
						mission.ship:AddEquip(Equipment.misc.shield_generator, mission.danger)

						mission.ship:GetComponent('CargoManager'):AddCommodity(Commodities.hydrogen, count)

						if mission.danger > 2 then
							mission.ship:AddEquip(Equipment.misc.shield_energy_booster)
						end

						if mission.danger > 3 then
							mission.ship:AddEquip(Equipment.misc.laser_cooling_booster)
						end

						_setupHooksForMission(mission)
						mission.shipstate = 'docked'
					end
				else	-- too late
					mission.status = 'FAILED'
				end
			else
				if not mission.ship:exists() then
					mission.ship = nil
					if mission.due < Game.time then
						mission.status = 'FAILED'
					end
				end
			end
		end
	end
end

local onLeaveSystem = function (ship)
	if ship:IsPlayer() then
		nearbysystems = nil
		planets = nil
	end
end

local onShipDocked = function (ship, station)
	for ref,mission in pairs(missions) do
		if ship:IsPlayer() then
			local oldReputation = Character.persistent.player.reputation
			if mission.status == 'COMPLETED' and
			   mission.backstation == station.path then
				local text = string.interp(flavours[mission.flavour].successmsg, {
					target	= mission.target,
					cash	= Format.Money(mission.reward,false),
				})
				Comms.ImportantMessage(text, mission.client.name)
				ship:AddMoney(mission.reward)
				Character.persistent.player.reputation = Character.persistent.player.reputation + 8
				mission:Remove()
				missions[ref] = nil
			elseif mission.status == 'FAILED' then
				local text
				if mission.notplayer == 'TRUE' then
					text = string.interp(flavours[mission.flavour].failuremsg2, {
						target	= mission.target,
					})
				else
					text = string.interp(flavours[mission.flavour].failuremsg, {
						target	= mission.target,
					})
				end
				Comms.ImportantMessage(text, mission.client.name)
				Character.persistent.player.reputation = Character.persistent.player.reputation - 8
				mission:Remove()
				missions[ref] = nil
			end
			Event.Queue("onReputationChanged", oldReputation, Character.persistent.player.killcount,
				Character.persistent.player.reputation, Character.persistent.player.killcount)
		else
			-- Fail occurs when mission ship either lands or jumps,
			-- after taking off at said mission time point. Spawned
			-- docked ship will trigger an onShipDocked event, thus
			-- check mission.due
			if mission.ship == ship and mission.due < Game.time then
				mission.status = 'FAILED'
			end
		end
	end
end

local onShipUndocked = function (ship, station)
	if ship:IsPlayer() then return end -- not interested in player, yet

	for ref,mission in pairs(missions) do
		if mission.status == 'ACTIVE' and
		   mission.ship == ship then
			planets = Space.GetBodies("Planet")
			if #planets == 0 then
				ship:AIFlyTo(station)
				mission.shipstate = 'outbound'
			else
				local planet = Engine.rand:Integer(1,#planets)

				mission.ship:AIEnterMediumOrbit(planets[planet])
				mission.shipstate = 'flying'

				table.remove(planets, planet)
			end
			return
		end
	end
end

local onAICompleted = function (ship, ai_error)
	for ref,mission in pairs(missions) do
		if mission.status == 'ACTIVE' and
		   mission.ship == ship then
			if mission.shipstate == 'outbound' then
				local systems = Game.system:GetNearbySystems(ship.hyperspaceRange, function (s) return #s:GetStationPaths() > 0 end)
				if #systems == 0 then return end
				local system = systems[Engine.rand:Integer(1,#systems)]

				mission.shipstate = 'inbound'
				ship:HyperjumpTo(system.path)
			-- the only other states are flying and inbound, and there is no AI to complete for inbound
			elseif ai_error == 'NONE' then
				Timer:CallAt(Game.time + 60 * 60 * 8, function ()
					if mission.ship:exists() then
						local stations = Space.GetBodies(function (body) return body:isa("SpaceStation") end)
						if #stations == 0 then return end
						local station = stations[Engine.rand:Integer(1,#stations)]

						mission.ship:AIDockWith(station)
					end
				end)
			else
				if #planets > 0 then
					local planet = Engine.rand:Integer(1,#planets)

					mission.ship:AIEnterMediumOrbit(planets[planet])

					table.remove(planets, planet)
				else
					mission.ship:AIFlyTo(Space.GetBody(mission.location.bodyIndex))
					mission.shipstate = 'outbound'
				end
			end
		end
	end
end

local onUpdateBB = function (station)
	for ref,ad in pairs(ads) do
		if ad.timeout < Game.time then
			ad.station:RemoveAdvert(ref)
		end
	end
	if Engine.rand:Integer(4*24*60*60) < 60*60 then -- roughly once every four days
		makeAdvert(station)
	end
end

local onReputationChanged = function (oldRep, oldKills, newRep, newKills)
	for ref,ad in pairs(ads) do
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

	if not loaded_data or not loaded_data.ads then return end

	for k,ad in pairs(loaded_data.ads) do
		placeAdvert(ad.station, ad)
	end

	missions = loaded_data.missions

	loaded_data = nil
end

local onGameEnd = function ()
	nearbysystems = nil
end

local function buildMissionDescription(mission)
	local ui = require 'pigui'
	local desc = {}
	local dist = Game.system and string.format("%.2f", Game.system:DistanceTo(mission.location)) or "???"

	desc.description = flavours[mission.flavour].introtext:interp({
		name   = mission.client.name,
		target = mission.target,
		system = mission.location:GetStarSystem().name,
		cash   = ui.Format.Money(mission.reward,false),
		dist  = dist
	})

	desc.details = {
		{ l.TARGET_NAME, mission.target },
		{ l.SPACEPORT, mission.location:GetSystemBody().name },
		{ l.SYSTEM, ui.Format.SystemPath(mission.location) },
		{ l.DISTANCE, dist.." "..lc.UNIT_LY },
		false,
		{ l.SHIP, mission.shipname },
		{ l.SHIP_ID, mission.shipregid },
		{ l.TARGET_WILL_BE_LEAVING_SPACEPORT_AT, ui.Format.Date(mission.due) }
	}

	desc.location = mission.location
	desc.returnLocation = mission.backstation
	desc.client = mission.client

	return desc
end

local serialize = function ()
	return { ads = ads, missions = missions }
end

local unserialize = function (data)
	loaded_data = data
	for k,mission in pairs(loaded_data.missions) do
		if mission.ship and
		   mission.ship:exists() and
		   mission.timer == 'SET' then
			Timer:CallAt(mission.due, function () if mission.ship:exists() then mission.ship:Undock()
				mission.timer = nil end end)
		end
	end
end

Event.Register("onCreateBB", onCreateBB)
Event.Register("onGameStart", onGameStart)
Event.Register("onEnterSystem", onEnterSystem)
Event.Register("onLeaveSystem", onLeaveSystem)
Event.Register("onShipDestroyed", onShipDestroyed)
Event.Register("onShipUndocked", onShipUndocked)
Event.Register("onAICompleted", onAICompleted)
Event.Register("onShipDocked", onShipDocked)
Event.Register("onShipHit", onShipHit)
Event.Register("onUpdateBB", onUpdateBB)
Event.Register("onGameEnd", onGameEnd)
Event.Register("onReputationChanged", onReputationChanged)

Mission.RegisterType('Assassination',l.ASSASSINATION, buildMissionDescription)

Serializer:Register("Assassination", serialize, unserialize)
