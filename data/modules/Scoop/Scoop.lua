-- Copyright © 2008-2024 Pioneer Developers. See AUTHORS.txt for details
-- Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

local Game = require 'Game'
local Lang = require 'Lang'
local Ship = require 'Ship'
local Comms = require 'Comms'
local Event = require 'Event'
local Space = require 'Space'
local Timer = require 'Timer'
local Engine = require 'Engine'
local Format = require 'Format'
local Mission = require 'Mission'
local MissionUtils = require 'modules.MissionUtils'
local ShipDef = require 'ShipDef'
local Character = require 'Character'
local Equipment = require 'Equipment'
local Serializer = require 'Serializer'

local CommodityType = require 'CommodityType'
local Commodities   = require 'Commodities'

local utils = require 'utils'

local l = Lang.GetResource("module-scoop")
local lc = Lang.GetResource("ui-core")

local AU = 149597870700.0
local LEGAL = 1
local ILLEGAL = 2

local mission_reputation = 1
local mission_time = 14*24*60*60
local max_dist = 20 * AU

local ads = {}
local missions = {}

local rescue_capsule = CommodityType.RegisterCommodity("rescue_capsule", {
	l10n_key = "RESCUE_CAPSULE",
	l10n_resource = "module-scoop",
	price = 500,
	icon_name = "Default",
	model_name = "escape_pod",
	mass = 1,
	purchasable = false
})

local rocket_launchers = CommodityType.RegisterCommodity("rocket_launchers", {
	l10n_key = "ROCKET_LAUNCHERS",
	l10n_resource = "module-scoop",
	price = 500,
	icon_name = "Default",
	mass = 1,
	purchasable = false
})

local detonators = CommodityType.RegisterCommodity("detonators", {
	l10n_key = "DETONATORS",
	l10n_resource = "module-scoop",
	price = 250,
	icon_name = "Default",
	mass = 1,
	purchasable = false
})

local nuclear_missile = CommodityType.RegisterCommodity("nuclear_missile", {
	l10n_key = "NUCLEAR_MISSILE",
	l10n_resource = "module-scoop",
	price = 1250,
	icon_name = "Default",
	model_name = "missile",
	mass = 1,
	purchasable = false
})

-- Useless waste that the player has to sort out
local toxic_waste = CommodityType.RegisterCommodity("toxic_waste", {
	l10n_key = "TOXIC_WASTE",
	l10n_resource = "module-scoop",
	price = -50,
	icon_name = "Default",
	mass = 1,
	purchasable = false
})

local spoiled_food = CommodityType.RegisterCommodity("spoiled_food", {
	l10n_key = "SPOILED_FOOD",
	l10n_resource = "module-scoop",
	price = -10,
	icon_name = "Default",
	mass = 1,
	purchasable = false
})

local unknown = CommodityType.RegisterCommodity("unknown", {
	l10n_key = "UNKNOWN",
	l10n_resource = "module-scoop",
	price = -5,
	icon_name = "Default",
	mass = 1,
	purchasable = false
})

local rescue_capsules = {
	rescue_capsule
}

local weapons = {
	rocket_launchers,
	detonators,
	nuclear_missile
}

local waste = {
	toxic_waste,
	spoiled_food,
	unknown,
	Commodities.radioactives,
	Commodities.rubbish
}

local flavours = {
	{
		id = "LEGAL_GOODS",
		cargo_type = nil,
		reward = -500,
		amount = 20,
	},
	{
		id = "ILLEGAL_GOODS",
		cargo_type = nil,
		reward = -1000,
		amount = 10,
	},
	{
		id = "RESCUE",
		cargo_type = rescue_capsules,
		reward = 750,
		amount = 4,
		return_to_station = true,
	},
	{
		id = "ARMS_DEALER",
		cargo_type = weapons,
		reward = 1000,
		amount = 1,
		deliver_to_ship = true,
	},
}

-- Sort goods, legal and illegal
local sortGoods = function (goods)
	local legal_goods = {}
	local illegal_goods = {}
	local system = Game.system

	for _, e in pairs(goods) do
		if e.purchasable then
			if system:IsCommodityLegal(e.name) then
				table.insert(legal_goods, e)
			else
				table.insert(illegal_goods, e)
			end
		end
	end

	return legal_goods, illegal_goods
end

-- Returns the number of flavours of the given string (assuming first flavour has suffix '_1').
local getNumberOfFlavours = function (str)
	local num = 1

	while l:get(str .. "_" .. num) do
		num = num + 1
	end

	return num - 1
end

-- Create a debris field in a random distance to a system body
local spawnDebris = function (debris, amount, sbody, min, max, lifetime)
	local list = {}
	local cargo
	local body = Space.GetBody(sbody:GetSystemBody().index)

	for i = 1, Engine.rand:Integer(math.ceil(amount / 4), amount) do
		cargo = debris[Engine.rand:Integer(1, #debris)]
		body = Space.SpawnCargoNear(cargo, body, min, max, lifetime)
		if i > 1 then body:SetVelocity(list[1].body:GetVelocity()) end
		table.insert(list, { cargo = cargo, body = body })
		min = 10
		max = 1000
	end

	-- add some useless waste
	for i = 1, Engine.rand:Integer(1, 9) do
		cargo = waste[Engine.rand:Integer(1, #waste)]
		body = Space.SpawnCargoNear(cargo, body, min, max, lifetime)
		body:SetVelocity(list[1].body:GetVelocity())
	end

	return list
end

-- Create a couple of police ships
local spawnPolice = function (station)
	local ship
	local police = {}
	local shipdef = ShipDef[Game.system.faction.policeShip]

	for i = 1, 2 do
		ship = Space.SpawnShipDocked(shipdef.id, station)
		ship:SetLabel(lc.POLICE)
		ship:AddEquip(Equipment.laser.pulsecannon_1mw)
		table.insert(police, ship)
		if station.type == "STARPORT_SURFACE" then
			ship:AIEnterLowOrbit(Space.GetBody(station:GetSystemBody().parent.index))
		end
	end

	Timer:CallAt(Game.time + 5, function ()
		for _, s in pairs(police) do
			s:AIKill(Game.player)
		end
	end)

	return police
end

-- Returns a random system close to the players location
local nearbySystem = function ()
	local dist = 5
	local systems = {}

	while #systems < 1 do
		systems = Game.system:GetNearbySystems(dist)
		dist = dist + 5
	end

	return systems[Engine.rand:Integer(1, #systems)].path
end

-- Create a ship in orbit
local spawnClientShip = function (star, ship_label)
	local shipdefs = utils.build_array(utils.filter(
		function (k, def)
			return def.tag == "SHIP" and def.hyperdriveClass > 0 and def.equipSlotCapacity["scoop"] > 0
		end,
		pairs(ShipDef)))
	local shipdef = shipdefs[Engine.rand:Integer(1, #shipdefs)]

	local radius = star:GetSystemBody().radius
	local min, max
	if star:GetSystemBody().type == "WHITE_DWARF" then
		min = radius * 30
		max = radius * 40
	else
		min = radius * 3.5
		max = radius * 4.5
	end

	local ship = Space.SpawnShipOrbit(shipdef.id, Space.GetBody(star:GetSystemBody().index), min, max)

	ship:SetLabel(ship_label)
	ship:AddEquip(Equipment.hyperspace["hyperdrive_" .. shipdef.hyperdriveClass])
	ship:AddEquip(Equipment.laser.pulsecannon_2mw)
	ship:AddEquip(Equipment.misc.shield_generator)

	return ship
end

local removeMission = function (mission, ref)
	local oldReputation = Character.persistent.player.reputation
	local sender = mission.client_ship and mission.ship_label or mission.client.name

	if mission.status == "COMPLETED" then
		Character.persistent.player.reputation = oldReputation + mission_reputation
		Game.player:AddMoney(mission.reward)
		Comms.ImportantMessage(l["SUCCESS_MSG_" .. mission.id], sender)
	elseif mission.status == "FAILED" then
		Character.persistent.player.reputation = oldReputation - mission_reputation
		Comms.ImportantMessage(l["FAILURE_MSG_" .. mission.id], sender)
	end
	Event.Queue("onReputationChanged", oldReputation, Character.persistent.player.killcount,
		Character.persistent.player.reputation, Character.persistent.player.killcount)

	if ref == nil then
		for r, m in pairs(missions) do
			if m == mission then ref = r break end
		end
	end
	mission:Remove()
	missions[ref] = nil
end

-- Cargo transfer to a ship
local transferCargo = function (mission, ref)
	Timer:CallEvery(9, function ()
		if not mission.client_ship then return true end

		if not mission.docking_in_progress and Game.player:DistanceTo(mission.client_ship) <= 5000 then
			mission.docking_in_progress = true
			Comms.ImportantMessage(l.DOCKING_INSTRUCTION, mission.ship_label)
		end

		if Game.player:DistanceTo(mission.client_ship) <= 100 then

			---@type CargoManager
			local cargoMgr = Game.player:GetComponent('CargoManager')
			---@type CargoManager
			local clientCargo = mission.client_ship:GetComponent('CargoManager')

			-- unload mission cargo
			for i, e in pairs(mission.debris) do
				if e.body == nil then
					if cargoMgr:RemoveCommodity(e.cargo, 1) == 1 then
						clientCargo:AddCommodity(e.cargo, 1)
						mission.debris[i] = nil
						mission.amount = mission.amount - 1
					end
				end
			end

			if mission.amount == 0 then
				mission.status = "COMPLETED"
			elseif mission.destination == nil then
				mission.status = "FAILED"
			end
		end

		if mission.status == "COMPLETED" or mission.status == "FAILED" then
			local ship = mission.client_ship
			mission.client_ship = nil
			removeMission(mission, ref)
			ship:HyperjumpTo(nearbySystem())
		end
	end)
end

local isQualifiedFor = function(reputation, ad)
	return reputation > (ad.reward/100) or false
end

local onDelete = function (ref)
	ads[ref] = nil
end

local isEnabled = function (ref)
	return ads[ref] ~= nil and isQualifiedFor(Character.persistent.player.reputation, ads[ref])
end

local onChat = function (form, ref, option)
	local ad = ads[ref]
	local player = Game.player
	local debris, ship, radius, mindist, maxdist

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

	form:AddNavButton(ad.planet)

	if option == 0 then
		local introtext = string.interp(ad.introtext, {
			client = ad.client.name,
			shipid = ad.ship_label,
			star   = ad.star:GetSystemBody().name,
			planet = ad.planet:GetSystemBody().name,
			cash   = Format.Money(math.abs(ad.reward), false),
		})
		form:SetMessage(introtext)

	elseif option == 1 then
		form:SetMessage(l["WHY_NOT_YOURSELF_" .. ad.id])

	elseif option == 2 then
		form:SetMessage(string.interp(l["HOW_MUCH_TIME_" .. ad.id], { star = ad.star:GetSystemBody().name, date = Format.Date(ad.due) }))

	elseif option == 3 then
		if ad.reward > 0 and player:CountEquip(Equipment.misc.cargo_scoop) == 0 and player:CountEquip(Equipment.misc.multi_scoop) == 0 then
			form:SetMessage(l.YOU_DO_NOT_HAVE_A_SCOOP)
			form:RemoveNavButton()
			return
		end

		if ad.reward < 0 and player:GetMoney() < math.abs(ad.reward) then
			form:SetMessage(l.YOU_DO_NOT_HAVE_ENOUGH_MONEY)
			form:RemoveNavButton()
			return
		end

		form:RemoveAdvertOnClose()
		ads[ref] = nil

		radius = ad.planet:GetSystemBody().radius
		if ad.planet:GetSystemBody().superType == "ROCKY_PLANET" then
			mindist = radius * 2.5
			maxdist = radius * 3.5
		else
			mindist = radius * 25
			maxdist = radius * 35
		end
		debris = spawnDebris(ad.debris_type, ad.amount, ad.planet, mindist, maxdist, ad.due - Game.time)

		if ad.reward < 0 then player:AddMoney(ad.reward) end
		if ad.deliver_to_ship then
			ship = spawnClientShip(ad.star, ad.ship_label)
		end

		local mission = {
			type              = "Scoop",
			location          = ad.location,
			introtext         = ad.introtext,
			client            = ad.client,
			station           = ad.station.path,
			star              = ad.star,
			planet            = ad.planet,
			id                = ad.id,
			debris            = debris,
			amount            = #debris,
			reward            = ad.reward,
			due               = ad.due,
			return_to_station = ad.return_to_station,
			deliver_to_ship   = ad.deliver_to_ship,
			client_ship       = ship,
			ship_label        = ad.ship_label,
			destination       = debris[1].body
		}

		table.insert(missions, Mission.New(mission))
		form:SetMessage(l["ACCEPTED_" .. ad.id])
		form:RemoveNavButton()
		form:AddNavButton(debris[1].body)
		return
	end

	form:AddOption(l.WHY_NOT_YOURSELF, 1)
	form:AddOption(l.HOW_MUCH_TIME, 2)
	form:AddOption(l.REPEAT_THE_REQUEST, 0)
	form:AddOption(l.OK_AGREED, 3)
end

local getPlanets = function (system)
	local planets = {}

	for _, p in ipairs(system:GetBodyPaths()) do
		if p:GetSystemBody().superType == "ROCKY_PLANET" or p:GetSystemBody().superType == "GAS_GIANT" then
			table.insert(planets, p)
		end
	end
	return planets
end

local planets = nil

local makeAdvert = function (station)
	if planets == nil then planets = getPlanets(Game.system) end
	if #planets == 0 then return end

	if flavours[LEGAL].cargo_type == nil then
		flavours[LEGAL].cargo_type, flavours[ILLEGAL].cargo_type = sortGoods(Commodities)
	end

	local stars = Game.system:GetStars()
	local star = stars[Engine.rand:Integer(1, #stars)]
	local planet = planets[Engine.rand:Integer(1, #planets)]
	local dist = station:DistanceTo(Space.GetBody(planet:GetSystemBody().index))
	local flavour = flavours[Engine.rand:Integer(1, #flavours)]
	local time = Engine.rand:Number(mission_time)
	local due = time + MissionUtils.TravelTimeLocal(dist) * Engine.rand:Number(0.9, 1.1)
	local timeout = due/2 + Game.time -- timeout after half of the travel time
	local reward

	if flavour.reward < 0 then
		reward = flavour.reward * (1.15 - dist / max_dist) * Engine.rand:Number(0.9, 1.1)
	else
		reward = flavour.reward * (1 + dist / max_dist) * Engine.rand:Number(0.75, 1.25)
	end
	reward = utils.round(reward, 50)
	due = utils.round(due + Game.time, 3600)

	if #flavour.cargo_type > 0 and dist < max_dist and station:DistanceTo(Space.GetBody(star.index)) < max_dist then
		local ad = {
			station           = station,
			location          = planet,
			introtext         = l["INTROTEXT_" .. flavour.id],
			client            = Character.New(),
			star              = star.path,
			planet            = planet,
			id                = flavour.id,
			debris_type       = flavour.cargo_type,
			reward            = math.ceil(reward),
			amount            = flavour.amount,
			due               = due,
			timeout           = timeout,
			return_to_station = flavour.return_to_station,
			deliver_to_ship   = flavour.deliver_to_ship,
			ship_label        = flavour.deliver_to_ship and Ship.MakeRandomLabel() or nil
		}

		ad.desc = string.interp(l["ADTEXT_" .. flavour.id], { cash = Format.Money(ad.reward, false) })

		local ref = station:AddAdvert({
			title       = l["ADTITLE_" .. flavour.id],
			description = ad.desc,
			icon        = flavour.id == "RESCUE" and "searchrescue" or "haul",
			onChat      = onChat,
			onDelete    = onDelete,
			isEnabled   = isEnabled
		})
		ads[ref] = ad
	end
end

local onCreateBB = function (station)
	local num = Engine.rand:Integer(0, math.ceil(Game.system.population * Game.system.lawlessness))
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

-- 0..25% chance for police to notice you scooping illegal cargo
local onPlayerCargoChanged = function (comm, amount)
	if not Game.system then return end

	if Game.system:IsCommodityLegal(comm.name) or Game.player:IsDocked() then return end

	for ref, mission in pairs(missions) do
		if not mission.police and mission.location:IsSameSystem(Game.system.path) then
			if (1 - Game.system.lawlessness) > Engine.rand:Number(4) then
				local station = Game.player:FindNearestTo("SPACESTATION")
				if station then mission.police = spawnPolice(station) end
			end
		end
	end
end

-- The attacker could be a ship or the planet
-- If scooped or destroyed by self-destruction, attacker is nil
local onCargoDestroyed = function (body, attacker)
	for ref, mission in pairs(missions) do
		for i, e in pairs(mission.debris) do
			if body == e.body then
				e.body = nil
				if body == mission.destination then
					-- remove NavButton
					mission.destination = nil
				end
				if attacker and (mission.return_to_station or mission.deliver_to_ship) then
					mission.status = "FAILED"
				end
				if mission.destination == nil then
					for i, e in pairs(mission.debris) do
						if e.body ~= nil then
							-- set next target
							mission.destination = e.body
							break
						end
					end
				end
				break
			end
		end
	end
end

local onJettison = function (ship, cargo)
	if not ship:IsPlayer() then return end

	for ref, mission in pairs(missions) do
		if mission.reward > 0 and not mission.warning then
			for i, e in pairs(mission.debris) do
				if cargo == e.cargo then
					mission.warning = true
					Comms.ImportantMessage(l.WARNING, mission.client.name)
					break
				end
			end
		end
	end
end

local onShipHit = function (ship, attacker)
	if ship:IsPlayer() then return end
	if attacker == nil or not attacker:isa('Ship') then return end

	for ref, mission in pairs(missions) do
		if mission.police then
			for _, s in pairs(mission.police) do
				if s == ship then
					ship:AIKill(attacker)
					break
				end
			end
		elseif mission.client_ship == ship then
			ship:AIKill(attacker)
			break
		end
	end
end

local onShipDestroyed = function (ship, attacker)
	if ship:IsPlayer() then return end

	for ref, mission in pairs(missions) do
		if mission.police then
			for i, s in pairs(mission.police) do
				if s == ship then
					table.remove(mission.police, i)
					break
				end
			end
		elseif mission.client_ship == ship then
			mission.client_ship = nil
			local msg = string.interp(l.SHIP_DESTROYED, {
				shipid  = mission.ship_label,
				station = mission.station:GetSystemBody().name
			})
			Comms.ImportantMessage(msg, mission.client.name)
			break
		end
	end
end

local onShipDocked = function (player, station)
	if not player:IsPlayer() then return end

	for ref, mission in pairs(missions) do
		if mission.police then
			for _, s in pairs(mission.police) do
				if station.type == "STARPORT_SURFACE" then
					s:AIEnterLowOrbit(Space.GetBody(station:GetSystemBody().parent.index))
				else
					s:AIFlyTo(station)
				end
			end
		end

		if mission.return_to_station or mission.deliver_to_ship and not mission.client_ship and mission.station == station.path then
			---@type CargoManager
			local cargoMgr = player:GetComponent('CargoManager')

			-- unload mission cargo
			for i, e in pairs(mission.debris) do
				if e.body == nil then
					if cargoMgr:RemoveCommodity(e.cargo, 1) == 1 then
						mission.debris[i] = nil
						mission.amount = mission.amount - 1
					end
				end
			end
			if mission.amount == 0 then
				mission.status = "COMPLETED"
			elseif mission.destination == nil then
				mission.status = "FAILED"
			end

			if mission.status == "COMPLETED" or mission.status == "FAILED" then
				removeMission(mission, ref)
			end

		-- remove stale missions, if any
		-- all cargo related to flavour 1 and 2 scooped or destroyed
		elseif mission.reward < 0 and mission.destination == nil then
			mission:Remove()
			missions[ref] = nil
		end
	end
end

local onShipUndocked = function (player, station)
	if not player:IsPlayer() then return end

	for ref, mission in pairs(missions) do
		if mission.police then
			for _, s in pairs(mission.police) do
				s:AIKill(player)
			end
		end

		if mission.deliver_to_ship and not mission.in_progess then
			mission.in_progress = true
			transferCargo(mission, ref)
		end
	end
end

local getPopulatedPlanets = function (system)
	local planets = {}

	for _, p in ipairs(system:GetBodyPaths()) do
		if p:GetSystemBody().population > 0 then
			table.insert(planets, p)
		end
	end
	return planets
end

local onEnterSystem = function (ship)
	if not ship:IsPlayer() or Game.system.population == 0 then return end

	local planets = getPopulatedPlanets(Game.system)
	local num = Engine.rand:Integer(0, math.ceil(Game.system.population * Game.system.lawlessness))

	flavours[LEGAL].cargo_type, flavours[ILLEGAL].cargo_type = sortGoods(Commodities)

	-- spawn random cargo (legal or illegal goods)
	for i = 1, num do
		local planet = planets[Engine.rand:Integer(1, #planets)]
		local radius = planet:GetSystemBody().radius
		local flavour = flavours[Engine.rand:Integer(LEGAL, ILLEGAL)]
		local debris = flavour.cargo_type
		if #debris > 0 then
			spawnDebris(debris, flavour.amount, planet, radius * 1.2, radius * 3.5, mission_time)
		end
	end
end

local onLeaveSystem = function (ship)
	if ship:IsPlayer() then
		for ref, mission in pairs(missions) do
			mission.destination = nil
			mission.police = nil
			if mission.client_ship then
				mission.client_ship = nil
				mission.status = "FAILED"
			end
			for i, e in pairs(mission.debris) do
				e.body = nil
			end
		end
		planets = nil
		flavours[LEGAL].cargo_type = nil
		flavours[ILLEGAL].cargo_type = nil
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

local buildMissionDescription = function(mission)
	local ui = require 'pigui'
	local desc = {}

	desc.description = mission.introtext:interp({
		client = mission.client.name,
		shipid = mission.ship_label,
		star   = mission.star:GetSystemBody().name,
		planet = mission.planet:GetSystemBody().name,
		cash   = Format.Money(math.abs(mission.reward), false)
	})

	desc.details = {
		{ l.CLIENT, mission.client.name },
		{ l.SPACEPORT, mission.station:GetSystemBody().name },
		mission.client_ship and { l.SHIP, mission.client_ship.label } or false,
		false,
		{ l.DEADLINE, ui.Format.Date(mission.due) }
	}

	desc.client = mission.client
	desc.location = mission.destination or nil
	if mission.deliver_to_ship then
		desc.returnLocation = mission.client_ship or mission.station
	end

	return desc
end

local loaded_data

local onGameStart = function ()
	ads = {}
	missions = {}

	if loaded_data and loaded_data.ads then

		for k, ad in pairs(loaded_data.ads) do
			local ref = ad.station:AddAdvert({
				title       = l["ADTITLE_" .. ad.id],
				description = ad.desc,
				icon        = ad.id == "RESCUE" and "searchrescue" or "haul",
				onChat      = onChat,
				onDelete    = onDelete,
				isEnabled   = isEnabled
			})
			ads[ref] = ad
		end

		missions = loaded_data.missions

		loaded_data = nil

		for ref, mission in pairs(missions) do
			if mission.deliver_to_ship then
				mission.in_progress = true
				transferCargo(mission, ref)
			end
		end
	end

	Game.player:GetComponent('CargoManager'):AddListener('scoop-mission', onPlayerCargoChanged)
end

local onGameEnd = function ()
	planets = nil
	flavours[LEGAL].cargo_type = nil
	flavours[ILLEGAL].cargo_type = nil
end

local serialize = function ()
	return { ads = ads, missions = missions }
end

local unserialize = function (data)
	loaded_data = data
end

Event.Register("onCreateBB", onCreateBB)
Event.Register("onUpdateBB", onUpdateBB)
Event.Register("onShipDocked", onShipDocked)
Event.Register("onShipUndocked", onShipUndocked)
Event.Register("onShipHit", onShipHit)
Event.Register("onShipDestroyed", onShipDestroyed)
Event.Register("onJettison", onJettison)
Event.Register("onCargoDestroyed", onCargoDestroyed)
Event.Register("onEnterSystem", onEnterSystem)
Event.Register("onLeaveSystem", onLeaveSystem)
Event.Register("onGameStart", onGameStart)
Event.Register("onGameEnd", onGameEnd)
Event.Register("onReputationChanged", onReputationChanged)

Mission.RegisterType("Scoop", l.SCOOP, buildMissionDescription)

Serializer:Register("Scoop", serialize, unserialize)
