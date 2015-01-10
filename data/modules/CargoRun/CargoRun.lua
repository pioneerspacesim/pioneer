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
local max_cargo = 10
local max_cargo_wholesaler = 100

-- number of strings in module-cargorun
local num_pirate_taunts = 7
local num_success_msg = 3
local num_failure_msg = 4
local num_accepted = 4
local num_how_much_mass = 7
local num_why_so_much_money = 8
local num_urgency = 5
local num_deny = 6
local num_risk = 6
local num_wholesaler = 2
local num_adtext_local = 3
local num_intro_local = 3
local num_adtext = 2
local num_intro = 2

-- the custom cargo
aluminium_tubes = Equipment.EquipType.New({
	l10n_key = 'ALUMINIUM_TUBES', slots="cargo", price=50,
	capabilities={mass=1},
	purchasable=false, icon_name="Default",
	l10n_resource="module-cargorun"
})
art_objects = Equipment.EquipType.New({
	l10n_key = 'ART_OBJECTS', slots="cargo", price=200,
	capabilities={mass=1},
	purchasable=false, icon_name="Default",
	l10n_resource="module-cargorun"
})
clus = Equipment.EquipType.New({
	l10n_key = 'CLUS', slots="cargo", price=20,
	capabilities={mass=1},
	purchasable=false, icon_name="Default",
	l10n_resource="module-cargorun"
})
diamonds = Equipment.EquipType.New({
	l10n_key = 'DIAMONDS', slots="cargo", price=300,
	capabilities={mass=1},
	purchasable=false, icon_name="Default",
	l10n_resource="module-cargorun"
})
digesters = Equipment.EquipType.New({
	l10n_key = 'DIGESTERS', slots="cargo", price=10,
	capabilities={mass=1},
	purchasable=false, icon_name="Default",
	l10n_resource="module-cargorun"
})
electrical_appliances = Equipment.EquipType.New({
	l10n_key = 'ELECTRICAL_APPLIANCES', slots="cargo", price=150,
	capabilities={mass=1},
	purchasable=false, icon_name="Default",
	l10n_resource="module-cargorun"
})
explosives = Equipment.EquipType.New({
	l10n_key = 'EXPLOSIVES', slots="cargo", price=50,
	capabilities={mass=1},
	purchasable=false, icon_name="Default",
	l10n_resource="module-cargorun"
})
furniture = Equipment.EquipType.New({
	l10n_key = 'FURNITURE', slots="cargo", price=15,
	capabilities={mass=1},
	purchasable=false, icon_name="Default",
	l10n_resource="module-cargorun"
})
greenhouses = Equipment.EquipType.New({
	l10n_key = 'GREENHOUSES', slots="cargo", price=20,
	capabilities={mass=1},
	purchasable=false, icon_name="Default",
	l10n_resource="module-cargorun"
})
hazardous_substances = Equipment.EquipType.New({
	l10n_key = 'HAZARDOUS_SUBSTANCES', slots="cargo", price=100,
	capabilities={mass=1},
	purchasable=false, icon_name="Default",
	l10n_resource="module-cargorun"
})
machine_tools = Equipment.EquipType.New({
	l10n_key = 'MACHINE_TOOLS', slots="cargo", price=10,
	capabilities={mass=1},
	purchasable=false, icon_name="Default",
	l10n_resource="module-cargorun"
})
neptunium = Equipment.EquipType.New({
	l10n_key = 'NEPTUNIUM', slots="cargo", price=200,
	capabilities={mass=1},
	purchasable=false, icon_name="Default",
	l10n_resource="module-cargorun"
})
plutonium = Equipment.EquipType.New({
	l10n_key = 'PLUTONIUM', slots="cargo", price=200,
	capabilities={mass=1},
	purchasable=false, icon_name="Default",
	l10n_resource="module-cargorun"
})
semi_finished_products = Equipment.EquipType.New({
	l10n_key = 'SEMI_FINISHED_PRODUCTS', slots="cargo", price=10,
	capabilities={mass=1},
	purchasable=false, icon_name="Default",
	l10n_resource="module-cargorun"
})
spaceship_parts = Equipment.EquipType.New({
	l10n_key = 'SPACESHIP_PARTS', slots="cargo", price=250,
	capabilities={mass=1},
	purchasable=false, icon_name="Default",
	l10n_resource="module-cargorun"
})
titanium = Equipment.EquipType.New({
	l10n_key = 'TITANIUM', slots="cargo", price=150,
	capabilities={mass=1},
	purchasable=false, icon_name="Default",
	l10n_resource="module-cargorun"
})
tungsten = Equipment.EquipType.New({
	l10n_key = 'TUNGSTEN', slots="cargo", price=125,
	capabilities={mass=1},
	purchasable=false, icon_name="Default",
	l10n_resource="module-cargorun"
})
uranium = Equipment.EquipType.New({
	l10n_key = 'URANIUM', slots="cargo", price=175,
	capabilities={mass=1},
	purchasable=false, icon_name="Default",
	l10n_resource="module-cargorun"
})
quibbles = Equipment.EquipType.New({
	l10n_key = 'QUIBBLES', slots="cargo", price=1,
	capabilities={mass=1},
	purchasable=false, icon_name="Default",
	l10n_resource="module-cargorun"
})
wedding_dresses = Equipment.EquipType.New({
	l10n_key = 'WEDDING_DRESSES', slots="cargo", price=15,
	capabilities={mass=1},
	purchasable=false, icon_name="Default",
	l10n_resource="module-cargorun"
})

local chemical = {}
table.insert(chemical, digesters)
table.insert(chemical, hazardous_substances)

local mining = {}
table.insert(mining, clus)
table.insert(mining, explosives)

local hardware = {}
table.insert(hardware, aluminium_tubes)
table.insert(hardware, diamonds)
table.insert(hardware, hazardous_substances)
table.insert(hardware, machine_tools)
table.insert(hardware, neptunium)		-- 5
table.insert(hardware, plutonium)
table.insert(hardware, semi_finished_products)
table.insert(hardware, spaceship_parts)
table.insert(hardware, titanium)
table.insert(hardware, tungsten)
table.insert(hardware, uranium)

local infrastructure = {}
table.insert(infrastructure, clus)
table.insert(infrastructure, explosives)
table.insert(infrastructure, greenhouses)

local consumer_goods = {}
table.insert(consumer_goods, electrical_appliances)
table.insert(consumer_goods, furniture)
table.insert(consumer_goods, spaceship_parts)

local expensive = {} -- price >= 175
table.insert(expensive, art_objects)
table.insert(expensive, diamonds)
table.insert(expensive, neptunium)
table.insert(expensive, plutonium)
table.insert(expensive, spaceship_parts)
table.insert(expensive, uranium)

local crazy = {}
table.insert(crazy, quibbles)			-- 1
table.insert(crazy, wedding_dresses)		-- 2

local custom_cargo = {}
table.insert(custom_cargo, chemical)		-- 1
table.insert(custom_cargo, mining)		-- 2
table.insert(custom_cargo, hardware)		-- 3
table.insert(custom_cargo, infrastructure)	-- 4
table.insert(custom_cargo, consumer_goods)	-- 5
table.insert(custom_cargo, expensive)		-- 6
table.insert(custom_cargo, crazy)		-- 7

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
	local num, introtext

	form:Clear()

	if option == -1 then
		form:Close()
		return
	end

	local qualified = isQualifiedFor(Character.persistent.player.reputation, ad)

	form:SetFace(ad.client)

	if not qualified then
		form:SetMessage(l["DENY_" .. Engine.rand:Integer(1, num_deny)])
		return
	end

	if option == 0 then
		local sys   = ad.location:GetStarSystem()
		local sbody = ad.location:GetSystemBody()
		if ad.localdelivery then
			introtext = l["INTROTEXT_LOCAL_" .. ad.introtext]
		elseif ad.wholesaler then
			introtext = l["INTROTEXT_WHOLESALER_" .. ad.introtext]
		else
			introtext = l["INTROTEXT__" .. ad.branch .. "__" .. ad.cargotype] or l["INTROTEXT__" .. ad.branch] or l["INTROTEXT_" .. ad.introtext]
		end
		local introtext = string.interp(introtext, {
			name	  = ad.client.name,
			cash	  = Format.Money(ad.reward),
			cargoname = l[custom_cargo[ad.branch][ad.cargotype].l10n_key],
			starport  = sbody.name,
			system	  = sys.name,
			sectorx	  = ad.location.sectorX,
			sectory	  = ad.location.sectorY,
			sectorz	  = ad.location.sectorZ,
			dist	  = string.format("%.2f", ad.dist),
		})
		form:SetMessage(introtext)

	elseif option == 1 then
		num = custom_cargo[ad.branch][ad.cargotype].price >= 175 and num_why_so_much_money or (num_why_so_much_money - 2)
		if ad.localdelivery then
			form:SetMessage(l["WHYSOMUCHTEXT_" .. Engine.rand:Integer(1, num)])
		else
			form:SetMessage(l["WHYSOMUCHTEXT__" .. ad.branch .. "__" .. ad.cargotype] or l["WHYSOMUCHTEXT__" .. ad.branch] or l["WHYSOMUCHTEXT_" .. Engine.rand:Integer(1, num)])
		end

	elseif option == 2 then
		num = ad.cargo <= 3 and num_how_much_mass or (num_how_much_mass - 2)
		local howmuch = string.interp(l["HOWMUCH_" .. Engine.rand:Integer(1, num)], {
			cargo	  = ad.cargo,
			cargoname = l[custom_cargo[ad.branch][ad.cargotype].l10n_key],
		})
		form:SetMessage(howmuch)

	elseif option == 3 then
		if Game.player.freeCapacity < ad.cargo then
			form:SetMessage(l.YOU_DO_NOT_HAVE_ENOUGH_CARGO_SPACE_ON_YOUR_SHIP)
			return
		end
		Game.player:AddEquip(custom_cargo[ad.branch][ad.cargotype], ad.cargo, "cargo")
		form:RemoveAdvertOnClose()
		ads[ref] = nil
		local mission = {
			type		= "CargoRun",
			client		= ad.client,
			location	= ad.location,
			localdelivery	= ad.localdelivery,
			wholesaler	= ad.wholesaler,
			introtext	= ad.introtext,
			risk		= ad.risk,
			reward		= ad.reward,
			due		= ad.due,
			cargo		= ad.cargo,
			branch		= ad.branch,
			cargotype	= ad.cargotype,
		}
		table.insert(missions,Mission.New(mission))
		form:SetMessage(l["ACCEPTED_" .. Engine.rand:Integer(1, num_accepted)])
		return

	elseif option == 4 then
		local urgency
		local num = math.floor(ad.urgency * (num_urgency - 1)) + 1
		if ad.localdelivery then
			urgency = l["URGENCY_" .. num]
		else
			urgency = l["URGENCY__" .. ad.branch .. "__" .. ad.cargotype] or l["URGENCY__" .. ad.branch] or l["URGENCY_" .. num]
		end
		form:SetMessage(urgency .. Format.Date(ad.due))

	elseif option == 5 then
		local num = math.floor(ad.risk * (num_risk - 1)) + 1
		if ad.localdelivery then
			form:SetMessage(l["RISK_" .. num])
		else
			form:SetMessage(l["RISK__" .. ad.branch .. "__" .. ad.cargotype] or l["RISK__" .. ad.branch] or l["RISK_" .. num])
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

local nearbysystems

local makeAdvert = function (station, freeCargoSpace)
	local reward, due, location, nearbysystem, dist, nearbystations, cargo, adtext, introtext
	local branch, cargotype, risk
	local client = Character.New()
	local urgency = Engine.rand:Number(0, 1)
	local localdelivery = Engine.rand:Number(0, 1) > 0.5 and true or false
	local wholesaler = Engine.rand:Number(0, 1) > 0.75 and true or false

	if localdelivery then
		nearbysystem = Game.system
		nearbystations = findNearbyStations(station, 1000)
		if #nearbystations == 0 then return nil end
		if freeCargoSpace ~= nil then
			cargo = freeCargoSpace == 1 and 1 or Engine.rand:Integer(1, freeCargoSpace)
		else
			cargo = Engine.rand:Integer(1, max_cargo)
		end
		branch = Engine.rand:Integer(1, (#custom_cargo - 1)) -- no crazy cargo for local delivery
		cargotype = Engine.rand:Integer(1, #custom_cargo[branch])
		risk = 0 -- no risk for local delivery
		location, dist = table.unpack(nearbystations[Engine.rand:Integer(1,#nearbystations)])
		reward = typical_reward_local + (math.sqrt(dist) / 15000) * (1+urgency) + cargo
		due = Game.time + ((4*24*60*60) * (Engine.rand:Number(1.5,3.5) - urgency))
		introtext = Engine.rand:Integer(1, num_intro_local)
	else
		if nearbysystems == nil then
			nearbysystems = Game.system:GetNearbySystems(max_delivery_dist, function (s) return #s:GetStationPaths() > 0 end)
		end
		if #nearbysystems == 0 then return nil end
		nearbysystem = nearbysystems[Engine.rand:Integer(1,#nearbysystems)]
		dist = nearbysystem:DistanceTo(Game.system)
		nearbystations = nearbysystem:GetStationPaths()
		location = nearbystations[Engine.rand:Integer(1,#nearbystations)]
		if freeCargoSpace ~= nil then
			local approxFuelForJump = math.floor(dist / 10)
			cargo = (freeCargoSpace - approxFuelForJump) <= 1 and 1 or Engine.rand:Integer(1, (freeCargoSpace - approxFuelForJump))
		else
			if wholesaler then
				cargo = Engine.rand:Integer(max_cargo, max_cargo_wholesaler)
				introtext = Engine.rand:Integer(1, num_wholesaler)
			else
				cargo = Engine.rand:Integer(1, max_cargo)
				introtext = Engine.rand:Integer(1, num_intro)
			end
		end
		branch = Engine.rand:Integer(1, #custom_cargo)
		cargotype = Engine.rand:Integer(1, #custom_cargo[branch])
		risk = custom_cargo[branch][cargotype].price / 600 + Engine.rand:Number(0, 0.3) -- goods with price 300 have a risk of 0.5 to 0.8
		reward = (dist / max_delivery_dist) * typical_reward * (1+risk) * (1.5+urgency) * (1+cargo/100) * Engine.rand:Number(0.8,1.2)
		due = Game.time + ((dist / max_delivery_dist) * typical_travel_time * (1.5-urgency) * Engine.rand:Number(0.9,1.1))
	end

	local ad = {
		station		= station,
		client		= client,
		location	= location,
		localdelivery	= localdelivery,
		wholesaler	= wholesaler,
		introtext	= introtext,
		dist		= dist,
		due		= due,
		cargo		= cargo,
		branch		= branch,
		cargotype	= cargotype,
		risk		= risk,
		urgency		= urgency,
		reward		= reward,
		isfemale	= isfemale,
		faceseed	= Engine.rand:Integer(),
	}

	local sbody = ad.location:GetSystemBody()

	if localdelivery then
		adtext = l["ADTEXT_LOCAL_" .. Engine.rand:Integer(1, num_adtext_local)]
	elseif wholesaler then
		adtext = l.ADTEXT_WHOLESALER
	else
		adtext = l["ADTEXT__" .. branch .. "__" .. cargotype] or l["ADTEXT__" .. branch] or l["ADTEXT_" .. Engine.rand:Integer(1, num_adtext)]
	end
	ad.desc = string.interp(adtext, {
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
		if ad then
			local approxFuelForJump = math.floor(ad.dist / 10)
			if ((ad.localdelivery and ad.cargo <= freeCargoSpace) or (canHyperspace and ad.cargo <= (freeCargoSpace + approxFuelForJump))) then
				numAchievableJobs = numAchievableJobs + 1
			end
		end
	end
	-- try to make one job that matches the players free cargo space
	if numAchievableJobs == 0 and freeCargoSpace ~= 0 then
		makeAdvert(station, freeCargoSpace)
	end
end

local onUpdateBB = function (station)
	for ref,ad in pairs(ads) do
		if ad.localdelivery then
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
			local risk = mission.risk
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
				local pirate_greeting = string.interp(l["PIRATE_TAUNTS_"..Engine.rand:Integer(1,num_pirate_taunts)], {
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

local unloadCargo = function (mission)
	local cargotype = custom_cargo[mission.branch][mission.cargotype]
	local uc = Game.player:RemoveEquip(cargotype, mission.cargo, "cargo")
	if uc ~= mission.cargo then -- this is ugly
		local list = Game.player:GetEquip("cargo")
		for _, i in pairs(list) do
			if i.l10n_key == cargotype.l10n_key then
				Game.player:RemoveEquip(i, 1, "cargo")
				uc = uc + 1
				if uc == mission.cargo then break end
			end
		end
	end
	return uc
end

local onShipDocked = function (player, station)
	if not player:IsPlayer() then return end

	for ref,mission in pairs(missions) do
		if mission.location == station.path then
			local reward
			local oldReputation = Character.persistent.player.reputation
			local cargo = unloadCargo(mission)

			if mission.localdelivery then reputation = 1 else reputation = 1.5 end
			if Game.time <= mission.due and cargo == mission.cargo then
				Comms.ImportantMessage(l["SUCCESSMSG__" .. mission.branch .. "__" .. mission.cargotype] or l["SUCCESSMSG__" .. mission.branch] or l["SUCCESSMSG_" .. Engine.rand:Integer(1, num_success_msg)],
					mission.client.name)
				Character.persistent.player.reputation = Character.persistent.player.reputation + reputation
				player:AddMoney(mission.reward)
			else
				if cargo < mission.cargo then
					Comms.ImportantMessage(l.WHAT_IS_THIS, mission.client.name)
					player:AddMoney((cargo - mission.cargo) * custom_cargo[mission.branch][mission.cargotype].price) -- pay for the missing
					Comms.ImportantMessage(l.I_HAVE_DEBITED_YOUR_ACCOUNT, mission.client.name)
				else
					Comms.ImportantMessage(l["FAILUREMSG__" .. mission.branch .. "__" .. mission.cargotype] or l["FAILUREMSG__" .. mission.branch] or l["FAILUREMSG_" .. Engine.rand:Integer(1, num_failure_msg)],
						mission.client.name)
				end
				Character.persistent.player.reputation = Character.persistent.player.reputation - reputation
			end
			Event.Queue("onReputationChanged", oldReputation, Character.persistent.player.killcount,
				Character.persistent.player.reputation, Character.persistent.player.killcount)

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
	local introtext, danger
	local dist = Game.system and string.format("%.2f", Game.system:DistanceTo(mission.location)) or "???"
	if mission.localdelivery then
		introtext = l["INTROTEXT_LOCAL_" .. mission.introtext]
		danger = l["RISK_" .. math.floor(mission.risk * (num_risk - 1)) + 1]
	else
		if mission.wholesaler then
			introtext = l["INTROTEXT_WHOLESALER_" .. mission.introtext]
		else
			introtext = l["INTROTEXT__" .. mission.branch .. "__" .. mission.cargotype] or l["INTROTEXT__" .. mission.branch] or l["INTROTEXT_" .. mission.introtext]
		end
		danger = (l["RISK__" .. mission.branch .. "__" .. mission.cargotype] or l["RISK__" .. mission.branch] or l["RISK_" .. math.floor(mission.risk * (num_risk - 1)) + 1])
	end

	return ui:Grid(2,1)
		:SetColumn(0,{ui:VBox(10):PackEnd({ui:MultiLineText((introtext):interp({name = mission.client.name,
											cargoname = l[custom_cargo[mission.branch][mission.cargotype].l10n_key],
											starport = mission.location:GetSystemBody().name,
											system = mission.location:GetStarSystem().name,
											sectorx = mission.location.sectorX,
											sectory = mission.location.sectorY,
											sectorz = mission.location.sectorZ,
											cash = Format.Money(mission.reward),
											dist = dist})
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
													ui:Label(l[custom_cargo[mission.branch][mission.cargotype].l10n_key])
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
