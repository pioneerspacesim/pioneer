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
local Equipment = import("Equipment")
local ShipDef = import("ShipDef")
local Ship = import("Ship")
local utils = import("utils")

local InfoFace = import("ui/InfoFace")

local l = Lang.GetResource("module-cargorun")

-- Get the UI class
local ui = Engine.ui

-- don't produce missions for further than this many light years away
local max_delivery_dist = 15
-- typical time for travel to a system max_delivery_dist away
--	Irigi: ~ 4 days for in-system travel, the rest is FTL travel time
local typical_travel_time = (3 * max_delivery_dist + 4) * 24 * 60 * 60
-- typical reward for delivery to a system max_delivery_dist away
local typical_reward = 35 * max_delivery_dist
-- typical reward for local delivery
local typical_reward_local = 35
-- max cargo per trip
local max_cargo = 15
local max_cargo_wholesaler = 150

local num_pirate_taunts = 7
local num_deny = 6

-- the custom cargo
aluminium_tubes = Equipment.EquipType.New({
	l10n_key = 'ALUMINIUM_TUBES', slots="cargo", price=50,
	capabilities={mass=1},
	purchasable=false, icon_name="Narcotics",
	l10n_resource="module-cargorun"
})
art_objects = Equipment.EquipType.New({
	l10n_key = 'ART_OBJECTS', slots="cargo", price=200,
	capabilities={mass=1},
	purchasable=false, icon_name="Narcotics",
	l10n_resource="module-cargorun"
})
clus = Equipment.EquipType.New({
	l10n_key = 'CLUS', slots="cargo", price=20,
	capabilities={mass=1},
	purchasable=false, icon_name="Narcotics",
	l10n_resource="module-cargorun"
})
diamonds = Equipment.EquipType.New({
	l10n_key = 'DIAMONDS', slots="cargo", price=300,
	capabilities={mass=1},
	purchasable=false, icon_name="Narcotics",
	l10n_resource="module-cargorun"
})
digesters = Equipment.EquipType.New({
	l10n_key = 'DIGESTERS', slots="cargo", price=10,
	capabilities={mass=1},
	purchasable=false, icon_name="Narcotics",
	l10n_resource="module-cargorun"
})
electrical_appliances = Equipment.EquipType.New({
	l10n_key = 'ELECTRICAL_APPLIANCES', slots="cargo", price=150,
	capabilities={mass=1},
	purchasable=false, icon_name="Narcotics",
	l10n_resource="module-cargorun"
})
explosives = Equipment.EquipType.New({
	l10n_key = 'EXPLOSIVES', slots="cargo", price=50,
	capabilities={mass=1},
	purchasable=false, icon_name="Narcotics",
	l10n_resource="module-cargorun"
})
furniture = Equipment.EquipType.New({
	l10n_key = 'FURNITURE', slots="cargo", price=15,
	capabilities={mass=1},
	purchasable=false, icon_name="Narcotics",
	l10n_resource="module-cargorun"
})
greenhouses = Equipment.EquipType.New({
	l10n_key = 'GREENHOUSES', slots="cargo", price=20,
	capabilities={mass=1},
	purchasable=false, icon_name="Narcotics",
	l10n_resource="module-cargorun"
})
hazardous_substances = Equipment.EquipType.New({
	l10n_key = 'HAZARDOUS_SUBSTANCES', slots="cargo", price=100,
	capabilities={mass=1},
	purchasable=false, icon_name="Narcotics",
	l10n_resource="module-cargorun"
})
machine_tools = Equipment.EquipType.New({
	l10n_key = 'MACHINE_TOOLS', slots="cargo", price=10,
	capabilities={mass=1},
	purchasable=false, icon_name="Narcotics",
	l10n_resource="module-cargorun"
})
neptunium = Equipment.EquipType.New({
	l10n_key = 'NEPTUNIUM', slots="cargo", price=200,
	capabilities={mass=1},
	purchasable=false, icon_name="Radioactive_waste",
	l10n_resource="module-cargorun"
})
plutonium = Equipment.EquipType.New({
	l10n_key = 'PLUTONIUM', slots="cargo", price=200,
	capabilities={mass=1},
	purchasable=false, icon_name="Radioactive_waste",
	l10n_resource="module-cargorun"
})
semi_finished_products = Equipment.EquipType.New({
	l10n_key = 'SEMI_FINISHED_PRODUCTS', slots="cargo", price=10,
	capabilities={mass=1},
	purchasable=false, icon_name="Narcotics",
	l10n_resource="module-cargorun"
})
spaceship_parts = Equipment.EquipType.New({
	l10n_key = 'SPACESHIP_PARTS', slots="cargo", price=250,
	capabilities={mass=1},
	purchasable=false, icon_name="Narcotics",
	l10n_resource="module-cargorun"
})
titanium = Equipment.EquipType.New({
	l10n_key = 'TITANIUM', slots="cargo", price=150,
	capabilities={mass=1},
	purchasable=false, icon_name="Metal_alloys",
	l10n_resource="module-cargorun"
})
tungsten = Equipment.EquipType.New({
	l10n_key = 'TUNGSTEN', slots="cargo", price=125,
	capabilities={mass=1},
	purchasable=false, icon_name="Metal_alloys",
	l10n_resource="module-cargorun"
})
uranium = Equipment.EquipType.New({
	l10n_key = 'URANIUM', slots="cargo", price=175,
	capabilities={mass=1},
	purchasable=false, icon_name="Radioactive_waste",
	l10n_resource="module-cargorun"
})

local custom_cargo = {}
table.insert(custom_cargo, aluminium_tubes)
table.insert(custom_cargo, art_objects)
table.insert(custom_cargo, clus)
table.insert(custom_cargo, diamonds)
table.insert(custom_cargo, digesters)
table.insert(custom_cargo, electrical_appliances)
table.insert(custom_cargo, explosives)
table.insert(custom_cargo, furniture)
table.insert(custom_cargo, greenhouses)
table.insert(custom_cargo, hazardous_substances)
table.insert(custom_cargo, machine_tools)
table.insert(custom_cargo, neptunium)
table.insert(custom_cargo, plutonium)
table.insert(custom_cargo, semi_finished_products)
table.insert(custom_cargo, spaceship_parts)
table.insert(custom_cargo, titanium)
table.insert(custom_cargo, tungsten)
table.insert(custom_cargo, uranium)

local flavours = {
	{
		urgency = 0,
		risk = 0,
		localdelivery = false,
		wholesaler = false,
	}, {
		urgency = 0.1,
		risk = 0,
		localdelivery = false,
		wholesaler = false,
	}, {
		urgency = 0.6,
		risk = 0,
		localdelivery = false,
		wholesaler = false,
	}, {
		urgency = 0.4,
		risk = 0.75,
		localdelivery = false,
		wholesaler = false,
	}, {
		urgency = 0.1,
		risk = 0.1,
		localdelivery = false,
		wholesaler = false,
	}, {
		urgency = 0.1,
		risk = 0,
		localdelivery = true,
		wholesaler = false,
	}, {
		urgency = 0.2,
		risk = 0,
		localdelivery = true,
		wholesaler = false,
	}, {
		urgency = 0.4,
		risk = 0,
		localdelivery = true,
		wholesaler = false,
	}, {
		urgency = 0.6,
		risk = 0,
		localdelivery = true,
		wholesaler = false,
	}, {
		urgency = 0.8,
		risk = 0,
		localdelivery = true,
		wholesaler = false,
	}, {
		urgency = 0.2,
		risk = 0.2,
		localdelivery = false,
		wholesaler = true,
	}, {
		urgency = 0.4,
		risk = 0.4,
		localdelivery = false,
		wholesaler = true,
	}
}

-- add strings to flavours
for i = 1,#flavours do
	local f = flavours[i]
	f.adtext        = l["FLAVOUR_" .. i-1 .. "_ADTEXT"]
	f.introtext     = l["FLAVOUR_" .. i-1 .. "_INTROTEXT"]
	f.whysomuchtext = l["FLAVOUR_" .. i-1 .. "_WHYSOMUCHTEXT"]
	f.howmuch	= l["FLAVOUR_" .. i-1 .. "_HOWMUCH"]
	f.successmsg    = l["FLAVOUR_" .. i-1 .. "_SUCCESSMSG"]
	f.failuremsg    = l["FLAVOUR_" .. i-1 .. "_FAILUREMSG"]
end

local ads = {}
local missions = {}

local isQualifiedFor = function(reputation, ad)
	return
		reputation >= 12 or
		ad.localdelivery or
		(ad.risk <  0.1 and ad.urgency <= 0.1) or
		(ad.risk <  0.5 and ad.urgency <= 0.5 and reputation >= 4) or
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
		local sbody = ad.location:GetSystemBody()

		local introtext = string.interp(flavours[ad.flavour].introtext, {
			name	  = ad.client.name,
			cash	  = Format.Money(ad.reward),
			cargoname = l[ad.cargotype.l10n_key],
			starport  = sbody.name,
			system	  = sys.name,
			sectorx	  = ad.location.sectorX,
			sectory	  = ad.location.sectorY,
			sectorz	  = ad.location.sectorZ,
			dist	  = string.format("%.2f", ad.dist),
		})
		form:SetMessage(introtext)

	elseif option == 1 then
		form:SetMessage(flavours[ad.flavour].whysomuchtext)

	elseif option == 2 then
		local howmuch = string.interp(flavours[ad.flavour].howmuch, {
			cargo	  = ad.cargo,
			cargoname = l[ad.cargotype.l10n_key],
		})

		form:SetMessage(howmuch)

	elseif option == 3 then
		if Game.player.freeCapacity < ad.cargo then
			form:SetMessage(l.YOU_DO_NOT_HAVE_ENOUGH_CARGO_SPACE_ON_YOUR_SHIP)
			return
		end

		Game.player:AddEquip(ad.cargotype, ad.cargo, "cargo")

		form:RemoveAdvertOnClose()

		ads[ref] = nil

		local mission = {
			type	  = "CargoRun",
			client	  = ad.client,
			location  = ad.location,
			risk	  = ad.risk,
			reward	  = ad.reward,
			due	  = ad.due,
			cargo	  = ad.cargo,
			cargotype = ad.cargotype,
			flavour	  = ad.flavour
		}

		table.insert(missions,Mission.New(mission))

		form:SetMessage(l.EXCELLENT_I_WILL_LET_THE_RECIPIENT_KNOW_YOU_ARE_ON_YOUR_WAY)

		return

	elseif option == 4 then
		form:SetMessage(l.IT_MUST_BE_DELIVERED_BY..Format.Date(ad.due))

	elseif option == 5 then
		if ad.risk <= 0.1 then
			form:SetMessage(l.NO)
		elseif ad.risk > 0.1 and ad.risk <= 0.3 then
			form:SetMessage(l.NOT_ANY_MORE_THAN_USUAL)
		elseif ad.risk > 0.3 and ad.risk <= 0.6 then
			form:SetMessage(l.THIS_IS_A_VALUABLE_CARGO_YOU_SHOULD_KEEP_YOUR_EYES_OPEN)
		elseif ad.risk > 0.6 and ad.risk <= 0.8 then
			form:SetMessage(l.IT_COULD_BE_DANGEROUS_YOU_SHOULD_MAKE_SURE_YOURE_ADEQUATELY_PREPARED)
		elseif ad.risk > 0.8 and ad.risk <= 1 then
			form:SetMessage(l.THIS_IS_VERY_RISKY_YOU_WILL_ALMOST_CERTAINLY_RUN_INTO_RESISTANCE)
		end
	end

	form:AddOption(l.WHY_SO_MUCH_MONEY, 1)
	form:AddOption(l.HOW_MUCH_MASS, 2)
	form:AddOption(l.HOW_SOON_MUST_IT_BE_DELIVERED, 4)
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

local findNearbyStations = function (station, minDist)
	local nearbystations = {}
	for _,s in ipairs(Game.system:GetStationPaths()) do
		if s ~= station.path then
			local dist = station:DistanceTo(Space.GetBody(s.bodyIndex))
			if dist >= minDist then
				table.insert(nearbystations, { s, dist })
			end
		end
	end
	return nearbystations
end

local getAvailCargo = function (min_price)
	local availcargo = {}
	for _, c in pairs(custom_cargo) do
		if c.price >= min_price and c.price <= (min_price + 100) then
			table.insert(availcargo, c)
		end
	end
	return availcargo
end

local nearbysystems

local makeAdvert = function (station, freeCargoSpace)
	local reward, due, location, nearbysystem, dist, nearbystations, cargo, cargotype
	local client = Character.New()
	local flavour = Engine.rand:Integer(1,#flavours)
	local urgency = flavours[flavour].urgency
	local risk = flavours[flavour].risk
	local availcargo = getAvailCargo(risk * 250)

	if #availcargo == 1 then
		cargotype = availcargo[1]
	else
		cargotype = availcargo[Engine.rand:Integer(1, #availcargo)]
	end
	if flavours[flavour].localdelivery then
		nearbysystem = Game.system
		nearbystations = findNearbyStations(station, 1000)
		if #nearbystations == 0 then return nil end
		if freeCargoSpace then
			if freeCargoSpace == 1 then
				cargo = 1
			else
				cargo = Engine.rand:Integer(1, freeCargoSpace)
			end
		else
			cargo = Engine.rand:Integer(1, max_cargo)
		end
		location, dist = table.unpack(nearbystations[Engine.rand:Integer(1,#nearbystations)])
		reward = typical_reward_local + (math.sqrt(dist) / 15000) * (1+urgency) + cargo
		due = Game.time + ((4*24*60*60) * (Engine.rand:Number(1.5,3.5) - urgency))
	else
		if nearbysystems == nil then
			nearbysystems = Game.system:GetNearbySystems(max_delivery_dist, function (s) return #s:GetStationPaths() > 0 end)
		end
		if #nearbysystems == 0 then return nil end
		nearbysystem = nearbysystems[Engine.rand:Integer(1,#nearbysystems)]
		dist = nearbysystem:DistanceTo(Game.system)
		nearbystations = nearbysystem:GetStationPaths()
		location = nearbystations[Engine.rand:Integer(1,#nearbystations)]
		if freeCargoSpace then
			local approxFuelForJump = math.floor(dist / 10)
			if (freeCargoSpace - approxFuelForJump) == 1 then
				cargo = 1
			else
				cargo = Engine.rand:Integer(1, (freeCargoSpace - approxFuelForJump))
			end
		else
			if flavours[flavour].wholesaler then
				cargo = Engine.rand:Integer(max_cargo, max_cargo_wholesaler)
			else
				cargo = Engine.rand:Integer(1, max_cargo)
			end
		end
		reward = (dist / max_delivery_dist) * typical_reward * (1+risk) * (1.5+urgency) * (1+cargo/100) * Engine.rand:Number(0.8,1.2)
		due = Game.time + ((dist / max_delivery_dist) * typical_travel_time * (1.5-urgency) * Engine.rand:Number(0.9,1.1))
	end

	local ad = {
		station		= station,
		flavour		= flavour,
		client		= client,
		location	= location,
		localdelivery	= flavours[flavour].localdelivery,
		wholesaler	= flavours[flavour].wholesaler,
		dist		= dist,
		due		= due,
		cargo		= cargo,
		cargotype	= cargotype,
		risk		= risk,
		urgency		= urgency,
		reward		= reward,
		isfemale	= isfemale,
		faceseed	= Engine.rand:Integer(),
	}

	local sbody = ad.location:GetSystemBody()

	ad.desc = string.interp(flavours[flavour].adtext, {
		system	= nearbysystem.name,
		cash	= Format.Money(ad.reward),
		starport = sbody.name,
	})

	local ref = station:AddAdvert({
		description = ad.desc,
		icon        = ad.urgency >=  0.8 and "haul_fast" or "haul",
		onChat      = onChat,
		onDelete    = onDelete,
		isEnabled   = isEnabled })
	ads[ref] = ad

	return ad
end

local onCreateBB = function (station)
	local num = Engine.rand:Integer(0, math.ceil(Game.system.population))
	local numAchievableJobs = 0
	local reputation = Character.persistent.player.reputation
	local canHyperspace = Game.player.maxHyperspaceRange > 0
	local freeCargoSpace = Game.player.freeCapacity

	for i = 1,num do
		local ad = makeAdvert(station)
		local approxFuelForJump = math.floor(ad.dist / 10)
		if ad and ((ad.localdelivery and ad.cargo <= freeCargoSpace) or (canHyperspace and ad.cargo <= (freeCargoSpace + approxFuelForJump))) then
			numAchievableJobs = numAchievableJobs + 1
		end
	end

	-- try to make one job that matches the players free cargo space
	if numAchievableJobs == 0 and freeCargoSpace ~= 0 then
		makeAdvert(station, freeCargoSpace)
	end
end

local onUpdateBB = function (station)
	for ref,ad in pairs(ads) do
		if flavours[ad.flavour].localdelivery then
			if ad.due < Game.time + 2*60*60*24 then -- two day timeout for locals
				ad.station:RemoveAdvert(ref)
			end
		else
			if ad.due < Game.time + 5*60*60*24 then -- five day timeout for inter-system
				ad.station:RemoveAdvert(ref)
			end
		end
	end
	if Engine.rand:Integer(12*60*60) < 60*60 then -- roughly once every twelve hours
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

			-- if there is some risk and still no ships, flip a tricoin
			if ships < 1 and risk >= 0.2 and Engine.rand:Integer(2) == 1 then ships = 1 end

			-- XXX hull mass is a bad way to determine suitability for role
			local shipdefs = utils.build_array(utils.filter(function (k,def) return def.tag == 'SHIP'
				and def.hyperdriveClass > 0 and def.hullMass <= 400 end, pairs(ShipDef)))
			if #shipdefs == 0 then return end

			local ship

			while ships > 0 do
				ships = ships-1

				if Engine.rand:Number(1) <= risk then
					local shipdef = shipdefs[Engine.rand:Integer(1,#shipdefs)]
					local default_drive = Equipment.hyperspace['hyperdrive_'..tostring(shipdef.hyperdriveClass)]

					local max_laser_size = shipdef.capacity - default_drive.capabilities.mass
					local laserdefs = utils.build_array(utils.filter(
						function (k,l) return l:IsValidSlot('laser_front') and l.capabilities.mass <= max_laser_size and l.l10n_key:find("PULSECANNON") end,
						pairs(Equipment.laser)
					))
					local laserdef = laserdefs[Engine.rand:Integer(1,#laserdefs)]

					ship = Space.SpawnShipNear(shipdef.id, Game.player, 50, 100)
					ship:SetLabel(Ship.MakeRandomLabel())
					ship:AddEquip(default_drive)
					ship:AddEquip(laserdef)
					ship:AIKill(Game.player)
				end
			end

			if ship then
				local pirate_greeting = string.interp(l["PIRATE_TAUNTS_"..Engine.rand:Integer(1,num_pirate_taunts)-1], {
					client = mission.client.name, location = mission.location:GetSystemBody().name,})
				Comms.ImportantMessage(pirate_greeting, ship.label)
			end
		end

		if mission.status == "ACTIVE" and Game.time > mission.due then
			mission.status = 'FAILED'
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

		if mission.location == station.path and Game.player:CountEquip(mission.cargotype, "cargo") >= mission.cargo then
			local reward
			if flavours[mission.flavour].localdelivery then
				reward = 1
			else
				reward = 1.5
			end

			local oldReputation = Character.persistent.player.reputation
			if Game.time > mission.due then
				Comms.ImportantMessage(flavours[mission.flavour].failuremsg, mission.client.name)
				Character.persistent.player.reputation = Character.persistent.player.reputation - reward
			else
				Comms.ImportantMessage(flavours[mission.flavour].successmsg, mission.client.name)
				player:AddMoney(mission.reward)
				Character.persistent.player.reputation = Character.persistent.player.reputation + reward
			end
			Event.Queue("onReputationChanged", oldReputation, Character.persistent.player.killcount,
				Character.persistent.player.reputation, Character.persistent.player.killcount)

			Game.player:RemoveEquip(mission.cargotype, mission.cargo, "cargo")

			mission:Remove()
			missions[ref] = nil

		elseif mission.status == "ACTIVE" and Game.time > mission.due then
			mission.status = 'FAILED'
		end

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

	if not loaded_data then return end

	for k,ad in pairs(loaded_data.ads) do
		local ref = ad.station:AddAdvert({
			description = ad.desc,
			icon        = ad.urgency >=  0.8 and "haul_fast" or "haul",
			onChat      = onChat,
			onDelete    = onDelete,
			isEnabled   = isEnabled })
		ads[ref] = ad
	end

	missions = loaded_data.missions

	loaded_data = nil
end

local onGameEnd = function ()
	nearbysystems = nil
end

local onClick = function (mission)
	local dist = Game.system and string.format("%.2f", Game.system:DistanceTo(mission.location)) or "???"

	if mission.risk <= 0.1 then
		danger = (l.NO)
	elseif mission.risk > 0.1 and mission.risk <= 0.3 then
		danger = (l.NOT_ANY_MORE_THAN_USUAL)
	elseif mission.risk > 0.3 and mission.risk <= 0.6 then
		danger = (l.THIS_IS_A_VALUABLE_CARGO_YOU_SHOULD_KEEP_YOUR_EYES_OPEN)
	elseif mission.risk > 0.6 and mission.risk <= 0.8 then
		danger = (l.IT_COULD_BE_DANGEROUS_YOU_SHOULD_MAKE_SURE_YOURE_ADEQUATELY_PREPARED)
	elseif mission.risk > 0.8 and mission.risk <= 1 then
		danger = (l.THIS_IS_VERY_RISKY_YOU_WILL_ALMOST_CERTAINLY_RUN_INTO_RESISTANCE)
	end

	return ui:Grid(2,1)
		:SetColumn(0,{ui:VBox(10):PackEnd({ui:MultiLineText((flavours[mission.flavour].introtext):interp({
														name   = mission.client.name,
														cargoname = l[mission.cargotype.l10n_key],
														starport = mission.location:GetSystemBody().name,
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
													ui:Label(l.SPACEPORT)
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
													ui:Label(l.CARGO)
												})
											})
											:SetColumn(1, {
												ui:VBox():PackEnd({
													ui:Label(l[mission.cargotype.l10n_key])
												})
											}),
										ui:Grid(2,1)
											:SetColumn(0, {
												ui:VBox():PackEnd({
													ui:Label(l.AMOUNT)
												})
											})
											:SetColumn(1, {
												ui:VBox():PackEnd({
													ui:Label(mission.cargo.."t")
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
													ui:MultiLineText(danger)
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
	return { ads = ads, missions = missions }
end

local unserialize = function (data)
	loaded_data = data
end

Event.Register("onCreateBB", onCreateBB)
Event.Register("onUpdateBB", onUpdateBB)
Event.Register("onEnterSystem", onEnterSystem)
Event.Register("onLeaveSystem", onLeaveSystem)
Event.Register("onShipDocked", onShipDocked)
Event.Register("onGameStart", onGameStart)
Event.Register("onGameEnd", onGameEnd)
Event.Register("onReputationChanged", onReputationChanged)

Mission.RegisterType('CargoRun',l.CARGORUN,onClick)

Serializer:Register("CargoRun", serialize, unserialize)
