-- Copyright © 2008-2019 Pioneer Developers. See AUTHORS.txt for details
-- Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

local Engine = import("Engine")
local Lang = import("Lang")
local Game = import("Game")
local Space = import("Space")
local Comms = import("Comms")
local Event = import("Event")
local Timer = import("Timer")
local Mission = import("Mission")
local Format = import("Format")
local Serializer = import("Serializer")
local Character = import("Character")
local NameGen = import("NameGen")
local Equipment = import("Equipment")
local ShipDef = import("ShipDef")
local Ship = import("Ship")
local utils = import("utils")

local InfoFace = import("ui/InfoFace")
local NavButton = import("ui/NavButton")

local l = Lang.GetResource("module-combat")

-- Get the UI class
local ui = Engine.ui

-- typical time for travel to a planet in a system 1ly away and back
local typical_hyperspace_time = 2 * 24 * 60 * 60
local typical_travel_time = 24 * 24 * 60 *60
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

	end

	form:AddOption(l.WHAT_ARE_THE_MISSION_OBJECTIVES, 1)
	form:AddOption(l.WILL_I_BE_IN_TROUBLE, 2)
	form:AddOption(l.IS_THERE_A_TIME_LIMIT, 3)
	form:AddOption(l.HOW_WILL_I_BE_PAID, 4)
	form:AddOption(l.PLEASE_REPEAT_THE_MISSION_DETAILS, 0)
	form:AddOption(l.OK_AGREED, 5)
end

local onDelete = function (ref)
	ads[ref] = nil
end

local isEnabled = function (ref)
	return isQualifiedFor(Character.persistent.player.reputation, Character.persistent.player.killcount, ads[ref])
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

local makeAdvert = function (station)
	local flavour, location, dist, reward, due, org
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
	due = Game.time + typical_travel_time * Engine.rand:Number(0.9, 1.1) + dist * typical_hyperspace_time * (1.5 - urgency) * Engine.rand:Number(0.9, 1.1)

	if Engine.rand:Number(1) > 0.5 then
		local nearbysystems = location:GetStarSystem():GetNearbySystems(10)
		if #nearbysystems ~= 0 then
			rendezvous = nearbysystems[Engine.rand:Integer(1, #nearbysystems)]
		end
	end

	local introtext = l["GREETING_" .. Engine.rand:Integer(1, getNumberOfFlavours("GREETING"))] .. " " .. l[flavour.id .. "_" .. Engine.rand:Integer(1, getNumberOfFlavours(flavour.id))]

	local ad = {
		station     = station,
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
	}

	ad.desc = string.interp(l["ADTEXT_" .. Engine.rand:Integer(1, getNumberOfFlavours("ADTEXT"))],
		{ system = location:GetStarSystem().name, cash = Format.Money(ad.reward, false), mission = l["MISSION_TYPE_" .. math.ceil(ad.dedication * NUMSUBTYPES)] })

	local ref = station:AddAdvert({
		description = ad.desc,
		icon        = "combat",
		onChat      = onChat,
		onDelete    = onDelete,
		isEnabled   = isEnabled})
	ads[ref] = ad
end

local onCreateBB = function (station)
	local num = Engine.rand:Integer(0, math.ceil(Game.system.population * 2 * Game.system.lawlessness) + 1)
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

local onShipDestroyed = function (ship, attacker)
	if ship:IsPlayer() then return end

	for ref, mission in pairs(missions) do
		for i, s in pairs(mission.mercenaries) do
			if s == ship then
				table.remove(mission.mercenaries, i)
				if not mission.complete and (#mission.mercenaries == 0 or mission.dedication <= ARMEDRECON) then
					mission.complete = true
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

local onFrameChanged = function (player)
	if not player:isa("Ship") or not player:IsPlayer() then return end

	for ref, mission in pairs(missions) do
		if player.frameBody and player.frameBody.path == mission.location and not mission.in_progress then
			mission.in_progress = true

			local ships
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
						ship = Space.SpawnShipNear(shipdef.id, player.frameBody, 100000, 150000)
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
				Timer:CallEvery(player.frameBody:GetPhysicalRadius()/1000, function ()
					if mission.complete then return true end -- already complete
					if player.frameBody and player.frameBody.path == mission.location then
						mission.complete = true
						Comms.ImportantMessage(l.MISSION_COMPLETE)
						return true
					end
				end)
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
						return def.tag == "SHIP" and def.hyperdriveClass > 1 and (def.shipClass == "medium_fighter" or def.shipClass == "medium_courier" or def.shipClass == "light_freighter")
					end,
					pairs(ShipDef)))
				local shipdef = shipdefs[Engine.rand:Integer(1, #shipdefs)]

				local ship = Space.SpawnShipNear(shipdef.id, Game.player, 50, 100)
				ship:SetLabel(Ship.MakeRandomLabel())
				ship:AddEquip(Equipment.hyperspace["hyperdrive_" .. tostring(shipdef.hyperdriveClass)])
				ship:AddEquip(Equipment.cargo.hydrogen, 5)
				ship:AddEquip(Equipment.laser.pulsecannon_2mw)
				ship:AddEquip(Equipment.misc.shield_generator)

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
	if loaded_data and loaded_data.ads then
		ads = {}
		missions = {}

		for k, ad in pairs(loaded_data.ads) do
			local ref = ad.station:AddAdvert({
				description = ad.desc,
				icon        = "combat",
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
	for _, f in pairs(flavours) do
		f.planets = nil
	end
end

local onClick = function (mission)
	local dist = Game.system and string.format("%.2f", Game.system:DistanceTo(mission.location)) or "???"
	local type = l["MISSION_TYPE_" .. math.ceil(mission.dedication * NUMSUBTYPES)]

	return ui:Grid(2,1)
		:SetColumn(0,{ui:VBox(10):PackEnd({ui:MultiLineText((mission.introtext):interp({
														name    = mission.client.name,
														org     = mission.org,
														cash    = Format.Money(mission.reward, false),
														area    = mission.location:GetSystemBody().name,
														system  = mission.location:GetStarSystem().name,
														sectorx = mission.location.sectorX,
														sectory = mission.location.sectorY,
														sectorz = mission.location.sectorZ,
														mission = type,
														dist    = dist})
										),
										ui:Margin(10),
										ui:Grid(2,1)
											:SetColumn(0, {
												ui:VBox():PackEnd({
													ui:Label(l.MISSION)
												})
											})
											:SetColumn(1, {
												ui:VBox():PackEnd({
													ui:MultiLineText(type)
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
										ui:Grid(2,1)
											:SetColumn(0, {
												ui:VBox():PackEnd({
													ui:Label(l.AREA)
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
													ui:Label(l.DISTANCE)
												})
											})
											:SetColumn(1, {
												ui:VBox():PackEnd({
													ui:Label(dist.." "..l.LY)
												})
											}),
										ui:Grid(2,1)
											:SetColumn(0, {
												ui:VBox():PackEnd({
													ui:Label(l.TIME_LIMIT)
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
													ui:Label(l.DANGER)
												})
											})
											:SetColumn(1, {
												ui:VBox():PackEnd({
													ui:MultiLineText(l:get("RISK_" .. math.ceil(mission.risk * (getNumberOfFlavours("RISK")))))
												})
											}),
										NavButton.New(l.SET_AS_TARGET, mission.location),
										ui:Margin(5),
										ui:Grid(2,1)
											:SetColumn(0, {
												ui:VBox():PackEnd({
													ui:Label(l.PAYMENT_LOCATION)
												})
											})
											:SetColumn(1, {
												ui:VBox():PackEnd({
													mission.rendezvous and ui:MultiLineText(mission.rendezvous:GetStarSystem().name.." ("..mission.rendezvous.sectorX..","..mission.rendezvous.sectorY..","..mission.rendezvous.sectorZ..")")
													or ui:MultiLineText(string.interp(l[mission.flavour.id .. "_LAND_THERE"], { org = mission.org }))
												})
											}),
										mission.rendezvous and NavButton.New(l.SET_AS_TARGET, mission.rendezvous),
		})})
		:SetColumn(1, {
			ui:VBox(10):PackEnd({
				InfoFace.New(mission.client),
				ui:Label(mission.org),
			})
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
Event.Register("onEnterSystem", onEnterSystem)
Event.Register("onFrameChanged", onFrameChanged)
Event.Register("onLeaveSystem", onLeaveSystem)
Event.Register("onShipDocked", onShipDocked)
Event.Register("onShipDestroyed", onShipDestroyed)
Event.Register("onGameStart", onGameStart)
Event.Register("onGameEnd", onGameEnd)
Event.Register("onReputationChanged", onReputationChanged)

Mission.RegisterType("Combat",l.COMBAT,onClick)

Serializer:Register("Combat", serialize, unserialize)
