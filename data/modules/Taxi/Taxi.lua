-- Copyright © 2008-2013 Pioneer Developers. See AUTHORS.txt for details
-- Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

-- Get the translator function
local t = Translate:GetTranslator()
-- Get the UI class
local ui = Engine.ui

-- don't produce missions for further than this many light years away
local max_taxi_dist = 40
-- typical time for travel to a system max_taxi_dist away
--	Irigi: ~ 4 days for in-system travel, the rest is FTL travel time
local typical_travel_time = (2.0 * max_taxi_dist + 4) * 24 * 60 * 60
-- typical reward for taxi service to a system max_taxi_dist away
local typical_reward = 75 * max_taxi_dist
-- max number of passengers per trip
local max_group = 10

local ads = {}
local missions = {}
local passengers = 0

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
	local taxi_flavours = Translate:GetFlavours('Taxi')
	local ad = ads[ref]

	form:Clear()

	if option == -1 then
		form:Close()
		return
	end

	if option == 0 then
		form:SetFace(ad.client)

		local sys   = ad.location:GetStarSystem()

		local introtext = string.interp(taxi_flavours[ad.flavour].introtext, {
			name     = ad.client.name,
			cash     = Format.Money(ad.reward),
			system   = sys.name,
			sectorx  = ad.location.sectorX,
			sectory  = ad.location.sectorY,
			sectorz  = ad.location.sectorZ,
			dist     = string.format("%.2f", ad.dist),
		})

		form:SetMessage(introtext)

	elseif option == 1 then
		local corporation = t('CORPORATIONS')[Engine.rand:Integer(1,#(t('CORPORATIONS')))]
		local whysomuch = string.interp(taxi_flavours[ad.flavour].whysomuch, {
			corp     = corporation,
		})

		form:SetMessage(whysomuch)

	elseif option == 2 then
		local howmany = string.interp(taxi_flavours[ad.flavour].howmany, {
			group  = ad.group,
		})

		form:SetMessage(howmany)

	elseif option == 3 then
		local capacity = Game.player:GetEquipSlotCapacity('CABIN')
		if capacity < ad.group or Game.player:GetEquipCount('CABIN', 'UNOCCUPIED_CABIN') < ad.group then
			form:SetMessage(t("You do not have enough cabin space on your ship."))
			form:AddOption(t('HANG_UP'), -1)
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

		form:SetMessage(t("Excellent."))
		form:AddOption(t('HANG_UP'), -1)

		return
	elseif option == 4 then
		if taxi_flavours[ad.flavour].single == 1 then
			form:SetMessage(t("I must be there before ")..Format.Date(ad.due))
		else
			form:SetMessage(t("We want to be there before ")..Format.Date(ad.due))
		end

	elseif option == 5 then
		form:SetMessage(taxi_flavours[ad.flavour].danger)
	end

	form:AddOption(t("Why so much money?"), 1)
	form:AddOption(t("How many of you are there?"), 2)
	form:AddOption(t("How soon you must be there?"), 4)
	form:AddOption(t("Will I be in any danger?"), 5)
	form:AddOption(t("Could you repeat the original request?"), 0)
	form:AddOption(t("Ok, agreed."), 3)
	form:AddOption(t('HANG_UP'), -1)
end

local onDelete = function (ref)
	ads[ref] = nil
end

local nearbysystems
local makeAdvert = function (station)
	local reward, due, location
	local taxi_flavours = Translate:GetFlavours('Taxi')
	local client = Character.New()
	local flavour = Engine.rand:Integer(1,#taxi_flavours)
	local urgency = taxi_flavours[flavour].urgency
	local risk = taxi_flavours[flavour].risk
	local group = 1
	if taxi_flavours[flavour].single == 0 then
		group = Engine.rand:Integer(2,max_group)
	end

	if nearbysystems == nil then
		nearbysystems = Game.system:GetNearbySystems(max_taxi_dist, function (s) return #s:GetStationPaths() > 0 end)
	end
	if #nearbysystems == 0 then return end
	location = nearbysystems[Engine.rand:Integer(1,#nearbysystems)]
	local dist = location:DistanceTo(Game.system)
	reward = ((dist / max_taxi_dist) * typical_reward * (group / 2) * (1+risk) * (1+3*urgency) * Engine.rand:Number(0.8,1.2))
	due = Game.time + ((dist / max_taxi_dist) * typical_travel_time * (1.5-urgency) * Engine.rand:Number(0.9,1.1))

	local ad = {
		station		= station,
		flavour		= flavour,
		client		= client,
		location	= location.path,
		dist            = dist,
		due		= due,
		group		= group,
		risk		= risk,
		urgency		= urgency,
		reward		= reward,
		isfemale	= isfemale,
		faceseed	= Engine.rand:Integer(),
	}

	ad.desc = string.interp(taxi_flavours[flavour].adtext, {
		system	= location.name,
		cash	= Format.Money(ad.reward),
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
	local taxi_flavours = Translate:GetFlavours('Taxi')
	for ref,ad in pairs(ads) do
		if ad.due < Game.time + 5*60*60*24 then
			ad.station:RemoveAdvert(ref)
		end
	end
	if Engine.rand:Integer(24*60*60) < 60*60 then -- roughly once every day
		makeAdvert(station)
	end
end

local onEnterSystem = function (player)
	if (not player:IsPlayer()) then return end
	local taxi_flavours = Translate:GetFlavours('Taxi')

	local syspath = Game.system.path

	for ref,mission in pairs(missions) do
		if not mission.status and mission.location:IsSameSystem(syspath) then
			local risk = taxi_flavours[mission.flavour].risk
			local ships = 0

			local riskmargin = Engine.rand:Number(-0.3,0.3) -- Add some random luck
			if risk >= (1 + riskmargin) then ships = 3
			elseif risk >= (0.7 + riskmargin) then ships = 2
			elseif risk >= (0.5 + riskmargin) then ships = 1
			end

			if ships < 1 and risk > 0 and Engine.rand:Integer(math.ceil(1/risk)) == 1 then ships = 1 end

			local shiptypes = ShipType.GetShipTypes('SHIP', function (t)
				local mass = t.hullMass
				return mass >= 80 and mass <= 200
			end)
			if #shiptypes == 0 then return end

			local ship

			while ships > 0 do
				ships = ships-1

				if Engine.rand:Number(1) <= risk then
					local shipid = shiptypes[Engine.rand:Integer(1,#shiptypes)]
					local shiptype = ShipType.GetShipType(shipid)
					local default_drive = shiptype.defaultHyperdrive

					local max_laser_size = shiptype.capacity - EquipType.GetEquipType(default_drive).mass
					local lasers = EquipType.GetEquipTypes('LASER', function (e,et)
						return et.mass <= max_laser_size and string.sub(e,0,11) == 'PULSECANNON'
					end)
					local laser = lasers[Engine.rand:Integer(1,#lasers)]

					ship = Space.SpawnShipNear(shipid, Game.player, 50, 100)
					ship:AddEquip(default_drive)
					ship:AddEquip(laser)
					ship:AddEquip('SHIELD_GENERATOR', math.ceil(risk * 3))
					if Engine.rand:Number(2) <= risk then
						ship:AddEquip('LASER_COOLING_BOOSTER')
					end
					if Engine.rand:Number(3) <= risk then
						ship:AddEquip('SHIELD_ENERGY_BOOSTER')
					end
					ship:AIKill(Game.player)
				end
			end

			if ship then
				local pirate_greeting = string.interp(t('PIRATE_TAUNTS')[Engine.rand:Integer(1,#(t('PIRATE_TAUNTS')))], { client = mission.client.name,})
				Comms.ImportantMessage(pirate_greeting, ship.label)
			end
		end

		if not mission.status and Game.time > mission.due then
			mission.status = 'FAILED'
			Comms.ImportantMessage(taxi_flavours[mission.flavour].wherearewe, mission.client.name)
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
			local taxi_flavours = Translate:GetFlavours('Taxi')

			if Game.time > mission.due then
				Comms.ImportantMessage(taxi_flavours[mission.flavour].failuremsg, mission.client.name)
			else
				Comms.ImportantMessage(taxi_flavours[mission.flavour].successmsg, mission.client.name)
				player:AddMoney(mission.reward)
			end

			remove_passengers(mission.group)

			mission:Remove()
			missions[ref] = nil
		end
	end
end

local onShipUndocked = function (player, station)
	if not player:IsPlayer() then return end
	local current_passengers = Game.player:GetEquipCount('CABIN', 'PASSENGER_CABIN')
	if current_passengers >= passengers then return end -- nothing changed, good

	for ref,mission in pairs(missions) do
		remove_passengers(mission.group)

		Comms.ImportantMessage(t("Hey!?! You are going to pay for this!!!"), mission.client.name)
		mission:Remove()
		missions[ref] = nil
	end
end

local loaded_data

local onGameStart = function ()
	ads = {}
	missions = {}
	passengers = 0

	if not loaded_data then return end

	for k,ad in pairs(loaded_data.ads) do
		local ref = ad.station:AddAdvert(ad.desc, onChat, onDelete)
		ads[ref] = ad
	end

	missions = loaded_data.missions
	passengers = loaded_data.passengers

	loaded_data = nil
end

local onGameEnd = function ()
	nearbysystems = nil
end

local onClick = function (mission)
	local taxi_flavours = Translate:GetFlavours('Taxi')
	local dist = Game.system:DistanceTo(mission.location)
	return ui:Grid(2,1)
		:SetColumn(0,{ui:VBox(10):PackEnd({ui:MultiLineText((taxi_flavours[mission.flavour].introtext):interp({
														name   = mission.client.name,
														system = mission.location:GetStarSystem().name,
														sectorx = mission.location.sectorX,
														sectory = mission.location.sectorY,
														sectorz = mission.location.sectorZ,
														cash   = Format.Money(mission.reward),
														dist  = string.format("%.2f", dist)})
										),
										ui:Grid(2,1)
											:SetColumn(0, {
												ui:VBox():PackEnd(ui:MultiLineText(t('taximissiondetail')))
											})
											:SetColumn(1, {
												ui:VBox():PackEnd({
													ui:Label(mission.start:GetStarSystem().name.." ("..mission.location.sectorX..","..mission.location.sectorY..","..mission.location.sectorZ..")"),
													ui:Label(mission.location:GetStarSystem().name.." ("..mission.location.sectorX..","..mission.location.sectorY..","..mission.location.sectorZ..")"),
													ui:Label(string.interp(taxi_flavours[mission.flavour].howmany, {group = mission.group})),
													ui:Label(taxi_flavours[mission.flavour].danger),
													ui:Label(Format.Date(mission.due)),
													ui:Margin(10),
													ui:Label(math.ceil(dist).." "..t("ly"))
												})
											})
		})})
		:SetColumn(1, {
			ui:VBox(10):PackEnd(UI.InfoFace.New(mission.client))
		})
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

Mission.RegisterType('Taxi','Taxi',onClick)

Serializer:Register("Taxi", serialize, unserialize)
