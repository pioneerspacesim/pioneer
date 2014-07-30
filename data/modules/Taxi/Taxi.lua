-- Copyright © 2008-2014 Pioneer Developers. See AUTHORS.txt for details
-- Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

local Engine = import("Engine")
local Lang = import("Lang")
local Game = import("Game")
local Space = import("Space")
local Comms = import("Comms")
local Event = import("Event")
local Mission = import("Mission")
local NameGen = import("NameGen")
local Format = import("Format")
local Serializer = import("Serializer")
local Character = import("Character")
local ShipDef = import("ShipDef")
local Ship = import("Ship")
local eq = import("Equipment")
local utils = import("utils")

local InfoFace = import("ui/InfoFace")

-- Get the language resource
local l = Lang.GetResource("module-taxi")

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

	if option == 0 then
		local sys   = ad.location:GetStarSystem()

		local introtext = string.interp(flavours[ad.flavour].introtext, {
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
	return isQualifiedFor(Character.persistent.player.reputation, ads[ref])
end

local nearbysystems
local makeAdvert = function (station)
	local reward, due, location
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

	ad.desc = string.interp(flavours[flavour].adtext, {
		system	= location.name,
		cash	= Format.Money(ad.reward),
	})

	local ref = station:AddAdvert({
		description = ad.desc,
		icon        = ad.urgency >=  0.8 and "taxi_urgent" or "taxi",
		onChat      = onChat,
		onDelete    = onDelete,
		isEnabled   = isEnabled})
	ads[ref] = ad
end

local onCreateBB = function (station)
	local num = Engine.rand:Integer(0, math.ceil(Game.system.population))
	for i = 1,num do
		makeAdvert(station)
	end
end

local onUpdateBB = function (station)
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

	local syspath = Game.system.path

	for ref,mission in pairs(missions) do
		if mission.status == "ACTIVE" and mission.location:IsSameSystem(syspath) then

			local risk = flavours[mission.flavour].risk
			local ships = 0

			local riskmargin = Engine.rand:Number(-0.3,0.3) -- Add some random luck
			if risk >= (1 + riskmargin) then ships = 3
			elseif risk >= (0.7 + riskmargin) then ships = 2
			elseif risk >= (0.5 + riskmargin) then ships = 1
			end

			if ships < 1 and risk > 0 and Engine.rand:Integer(math.ceil(1/risk)) == 1 then ships = 1 end

			-- XXX hull mass is a bad way to determine suitability for role
			local shipdefs = utils.build_array(utils.filter(function (k,def) return def.tag == 'SHIP'
				and def.hyperdriveClass > 0 and def.hullMass > 10 and def.hullMass <= 200 end, pairs(ShipDef)))
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

	if not loaded_data then return end

	for k,ad in pairs(loaded_data.ads) do
		local ref = ad.station:AddAdvert({
			description = ad.desc,
			icon        = ad.urgency >=  0.8 and "taxi_urgent" or "taxi",
			onChat      = onChat,
			onDelete    = onDelete,
			isEnabled   = isEnabled})
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
	local dist = Game.system and string.format("%.2f", Game.system:DistanceTo(mission.location)) or "???"
	return ui:Grid(2,1)
		:SetColumn(0,{ui:VBox(10):PackEnd({ui:MultiLineText((flavours[mission.flavour].introtext):interp({
														name   = mission.client.name,
														system = mission.location:GetStarSystem().name,
														sectorx = mission.location.sectorX,
														sectory = mission.location.sectorY,
														sectorz = mission.location.sectorZ,
														cash   = Format.Money(mission.reward),
														dist  = dist})
										),
										ui:Margin(10),
										ui:Grid(2,1)
											:SetColumn(0, {
												ui:VBox():PackEnd({
													ui:Label(l.FROM)
												})
											})
											:SetColumn(1, {
												ui:VBox():PackEnd({
													ui:MultiLineText(mission.start:GetStarSystem().name.." ("..mission.start.sectorX..","..mission.start.sectorY..","..mission.start.sectorZ..")")
												})
											}),
										ui:Grid(2,1)
											:SetColumn(0, {
												ui:VBox():PackEnd({
													ui:Label(l.TO)
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
													ui:Label(l.GROUP_DETAILS)
												})
											})
											:SetColumn(1, {
												ui:VBox():PackEnd({
													ui:MultiLineText(string.interp(flavours[mission.flavour].howmany, {group = mission.group}))
												})
											}),
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
													ui:Label(l.DANGER)
												})
											})
											:SetColumn(1, {
												ui:VBox():PackEnd({
													ui:MultiLineText(flavours[mission.flavour].danger)
												})
											}),
										ui:Margin(5),
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
		})})
		:SetColumn(1, {
			ui:VBox(10):PackEnd(InfoFace.New(mission.client))
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
Event.Register("onReputationChanged", onReputationChanged)

Mission.RegisterType('Taxi',l.TAXI,onClick)

Serializer:Register("Taxi", serialize, unserialize)
