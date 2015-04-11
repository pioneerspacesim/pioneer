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
local typical_travel_time = (2.5 * max_delivery_dist + 8) * 24 * 60 * 60
-- typical reward for delivery to a system max_delivery_dist away
local typical_reward = 35 * max_delivery_dist
-- typical reward for local delivery
local typical_reward_local = 35
-- max cargo per trip
local max_cargo = 10
local max_cargo_wholesaler = 100
-- factor for pickup missions
local pickup_factor = 1.75
-- the maximum price of the custom cargo
local max_price = 300

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
table.insert(hardware, neptunium)
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

local fluffy = {}
table.insert(fluffy, quibbles)

local wedding = {}
table.insert(wedding, wedding_dresses)

local art = {}
table.insert(art, art_objects)

local gems = {}
table.insert(gems, diamonds)

local radioactive = {}
table.insert(radioactive, neptunium)
table.insert(radioactive, plutonium)
table.insert(radioactive, uranium)

local centrifuges = {}
table.insert(centrifuges, aluminium_tubes)
table.insert(centrifuges, machine_tools)
table.insert(centrifuges, titanium)

local custom_cargo = {
	{ bkey = "CHEMICAL", goods = chemical, weight },
	{ bkey = "MINING", goods = mining, weight },
	{ bkey = "HARDWARE", goods = hardware, weight },
	{ bkey = "INFRASTRUCTURE", goods = infrastructure, weight },
	{ bkey = "CONSUMER_GOODS", goods = consumer_goods, weight },
	{ bkey = "EXPENSIVE", goods = expensive, weight },
	{ bkey = "FLUFFY", goods = fluffy, weight },
	{ bkey = "WEDDING" , goods = wedding, weight },
	{ bkey = "ART", goods = art, weight },
	{ bkey = "GEMS", goods = gems, weight },
	{ bkey = "RADIOACTIVE", goods = radioactive, weight },
	{ bkey = "CENTRIFUGES", goods = centrifuges, weight }
}

-- Each branch should have a probability weight proportional to its size
local custom_cargo_weight_sum = 0
for branch,branch_array in pairs(custom_cargo) do
	custom_cargo[branch].weight = #branch_array.goods
	custom_cargo_weight_sum = custom_cargo_weight_sum + #branch_array.goods
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

local getNumberOf = function (str)
	local num = 0

	while l[str .. "_" .. num + 1] do
		num = num + 1
	end
	return num
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
		form:SetMessage(l["DENY_" .. Engine.rand:Integer(1, getNumberOf("DENY"))])
		return
	end

	if option == 0 then
		local introtext = string.interp(ad.introtext, {
			name		= ad.client.name,
			cash		= Format.Money(ad.reward),
			cargoname	= l[ad.cargotype.l10n_key],
			starport	= ad.location:GetSystemBody().name,
			system		= ad.location:GetStarSystem().name,
			sectorx		= ad.location.sectorX,
			sectory		= ad.location.sectorY,
			sectorz		= ad.location.sectorZ,
			dom_starport	= ad.domicile:GetSystemBody().name,
			dom_system	= ad.domicile:GetStarSystem().name,
			dom_sectorx	= ad.domicile.sectorX,
			dom_sectory	= ad.domicile.sectorY,
			dom_sectorz	= ad.domicile.sectorZ,
			dist		= string.format("%.2f", ad.dist),
		})
		form:SetMessage(introtext)

	elseif option == 1 then
		local n = getNumberOf("WHYSOMUCH_" .. ad.branch)
		if n >= 1 then
			form:SetMessage(string.interp(l["WHYSOMUCH_" .. ad.branch .. "_" .. Engine.rand:Integer(1, n)], { cargoname = l[ad.cargotype.l10n_key] }))
		elseif ad.urgency >= 0.8 then
			form:SetMessage(string.interp(l["WHYSOMUCH_URGENT_" .. Engine.rand:Integer( 1, getNumberOf("WHYSOMUCH_URGENT"))], { cargoname = l[ad.cargotype.l10n_key] }))
		else
			form:SetMessage(string.interp(l["WHYSOMUCH_" .. Engine.rand:Integer( 1, getNumberOf("WHYSOMUCH"))], { cargoname = l[ad.cargotype.l10n_key] }))
		end

	elseif option == 2 then
		local howmuch
		if ad.wholesaler then
			howmuch = string.interp(l["HOWMUCH_WHOLESALER_" .. Engine.rand:Integer(1, getNumberOf("HOWMUCH_WHOLESALER"))], {
				cargo	  = ad.cargo,
				cargoname = l[ad.cargotype.l10n_key],
			})
		else
			local num = ad.cargo <= 3 and getNumberOf("HOWMUCH") or (getNumberOf("HOWMUCH") - 2) -- The last 2 strings contain "only"
			howmuch = string.interp(l["HOWMUCH_" .. Engine.rand:Integer(1, num)], {
				cargo	  = ad.cargo,
			})
		end
		form:SetMessage(howmuch)

	elseif option == 3 then
		if Game.player.freeCapacity < ad.cargo then
			form:SetMessage(l.YOU_DO_NOT_HAVE_ENOUGH_CARGO_SPACE_ON_YOUR_SHIP)
			return
		end
		local cargo_picked_up
		if not ad.pickup then
			Game.player:AddEquip(ad.cargotype, ad.cargo, "cargo")
			cargo_picked_up = true
		else
			cargo_picked_up = false
		end
		form:RemoveAdvertOnClose()
		ads[ref] = nil
		local mission = {
			type		= "CargoRun",
			domicile	= ad.domicile,
			client		= ad.client,
			location	= ad.location,
			localdelivery	= ad.localdelivery,
			wholesaler	= ad.wholesaler,
			pickup		= ad.pickup,
			cargo_picked_up	= cargo_picked_up,
			introtext	= ad.introtext,
			risk		= ad.risk,
			reward		= ad.reward,
			due		= ad.due,
			cargo		= ad.cargo,
			branch		= ad.branch,
			cargotype	= ad.cargotype,
		}
		table.insert(missions,Mission.New(mission))
		if ad.pickup then
			form:SetMessage(l["ACCEPTED_PICKUP_" .. Engine.rand:Integer(1, getNumberOf("ACCEPTED_PICKUP"))])
		else
			form:SetMessage(l["ACCEPTED_" .. Engine.rand:Integer(1, getNumberOf("ACCEPTED"))])
		end
		return

	elseif option == 4 then
		form:SetMessage(string.interp(l["URGENCY_" .. ad.branch .. "_" .. math.floor(ad.urgency * (getNumberOf("URGENCY_" .. ad.branch) - 1)) + 1]
			or l["URGENCY_" .. math.floor(ad.urgency * (getNumberOf("URGENCY") - 1)) + 1], { date = Format.Date(ad.due) }))

	elseif option == 5 then
		if ad.localdelivery then -- very low risk -> no specific text to give no confusing answer
			form:SetMessage(l.RISK_1)
		else
			local branch
			if ad.wholesaler then branch = "WHOLESALER" else branch = ad.branch end
			form:SetMessage(l["RISK_" .. branch .. "_" .. math.floor(ad.risk * (getNumberOf("RISK_" .. branch) - 1)) + 1] or l["RISK_" .. math.floor(ad.risk * (getNumberOf("RISK") - 1)) + 1])
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

local randomCargo = function()
	local accumulator = 0
	local r = Engine.rand:Integer(0,custom_cargo_weight_sum)

	for k,b in pairs(custom_cargo) do
		accumulator = b.weight + accumulator
		if r <= accumulator then
			return
				custom_cargo[k].bkey,
				custom_cargo[k].goods[Engine.rand:Integer(1, #custom_cargo[k].goods)]
		end
	end
	error("Oh, dear! This should not happen.")
end

local nearbysystems

local makeAdvert = function (station)
	local reward, due, location, nearbysystem, dist, nearbystations, cargo
	local risk, wholesaler, pickup, branch, cargotype, missiontype
	local client = Character.New()
	local urgency = Engine.rand:Number(0, 1)
	local localdelivery = Engine.rand:Number(0, 1) > 0.5 and true or false

	branch, cargotype = randomCargo()
	if localdelivery then
		nearbysystem = Game.system
		nearbystations = findNearbyStations(station, 1000)
		if #nearbystations == 0 then return nil end
		cargo = Engine.rand:Integer(1, max_cargo)
		risk = 0 -- no risk for local delivery
		wholesaler = false -- no local wholesaler delivery
		pickup = Engine.rand:Number(0, 1) > 0.75 and true or false
		location, dist = table.unpack(nearbystations[Engine.rand:Integer(1,#nearbystations)])
		reward = typical_reward_local + (math.sqrt(dist) / 15000) * (1+urgency) * (1+cargo/max_cargo)
		due = (4*24*60*60) + (24*60*60 * (dist / (1.49*10^11))) * (1.5 - urgency)
		if pickup then
			missiontype = "PICKUP_LOCAL"
			reward = reward * pickup_factor
			due = due * pickup_factor + Game.time
		else
			missiontype = "LOCAL"
			due = due + Game.time
		end
	else
		if nearbysystems == nil then
			nearbysystems = Game.system:GetNearbySystems(max_delivery_dist, function (s) return #s:GetStationPaths() > 0 end)
		end
		if #nearbysystems == 0 then return nil end
		nearbysystem = nearbysystems[Engine.rand:Integer(1,#nearbysystems)]
		dist = nearbysystem:DistanceTo(Game.system)
		nearbystations = nearbysystem:GetStationPaths()
		location = nearbystations[Engine.rand:Integer(1,#nearbystations)]
		wholesaler = Engine.rand:Number(0, 1) > 0.75 and true or false
		if wholesaler then
			cargo = Engine.rand:Integer(max_cargo, max_cargo_wholesaler)
			missiontype = "WHOLESALER"
			pickup = false
		else
			cargo = Engine.rand:Integer(1, max_cargo)
			pickup = Engine.rand:Number(0, 1) > 0.75 and true or false
			if pickup then
				missiontype = "PICKUP"
			else
				missiontype = branch
			end
		end
		risk = 0.75 * cargotype.price / max_price + Engine.rand:Number(0, 0.25) -- goods with price max_price have a risk of 0.75 to 1
		reward = (dist / max_delivery_dist) * typical_reward * (1+risk) * (1.5+urgency) * (1+cargo/max_cargo_wholesaler) * Engine.rand:Number(0.8,1.2)
		due = (dist / max_delivery_dist) * typical_travel_time * (1.5 - urgency)
		if pickup then
			reward = reward * pickup_factor
			due = due * pickup_factor + Game.time
		else
			due = due + Game.time
		end
	end

	local n = getNumberOf("INTROTEXT_" .. missiontype)
	local introtext
	if n >= 1 then
		introtext = l["INTROTEXT_" .. missiontype .. "_" .. Engine.rand:Integer(1, n)]
	else
		introtext = l["INTROTEXT_" .. Engine.rand:Integer(1, getNumberOf("INTROTEXT"))]
	end
	local ad = {
		station		= station,
		domicile	= station.path,
		client		= client,
		location	= location,
		localdelivery	= localdelivery,
		wholesaler	= wholesaler,
		pickup		= pickup,
		introtext	= introtext,
		dist		= dist,
		due		= due,
		cargo		= cargo,
		branch		= branch,
		cargotype	= cargotype,
		risk		= risk,
		urgency		= urgency,
		reward		= reward,
		faceseed	= Engine.rand:Integer(),
	}

	n = getNumberOf("ADTEXT_" .. missiontype)
	local adtext
	if n >= 1 then
		adtext = l["ADTEXT_" .. missiontype .. "_" .. Engine.rand:Integer(1, n)]
	else
		adtext = l["ADTEXT_" .. Engine.rand:Integer(1, getNumberOf("ADTEXT"))]
	end
	ad.desc = string.interp(adtext, {
		system	 = nearbysystem.name,
		cash	 = Format.Money(ad.reward),
		starport = ad.location:GetSystemBody().name,
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
	for i = 1,num do
		makeAdvert(station)
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

local pirate_ships = {}
local pirate_gripes_time
local pirate_switch_target
local escort_ships = {}
local escort_chatter_time
local escort_switch_target

local onEnterSystem = function (player)
	if (not player:IsPlayer()) then return end

	local syspath = Game.system.path

	for ref,mission in pairs(missions) do
		if mission.status == "ACTIVE" and ((mission.location:IsSameSystem(syspath) and not mission.pickup) or (mission.domicile:IsSameSystem(syspath) and mission.cargo_picked_up)) then
			local risk = mission.risk
			local pirates = 0

			local riskmargin = Engine.rand:Number(-0.3,0.3) -- Add some random luck
			if risk >= (1 + riskmargin) then pirates = 3
			elseif risk >= (0.7 + riskmargin) then pirates = 2
			elseif risk >= (0.5 + riskmargin) then pirates = 1
			end

			-- if there is some risk and still no pirates, flip a tricoin
			if pirates < 1 and risk >= 0.2 and Engine.rand:Integer(2) == 1 then pirates = 1 end

			-- XXX hull mass is a bad way to determine suitability for role
			local shipdefs = utils.build_array(utils.filter(function (k,def) return def.tag == 'SHIP'
				and def.hyperdriveClass > 0 and def.hullMass <= 400 end, pairs(ShipDef)))
			if #shipdefs == 0 then return end

			local pirate

			while pirates > 0 do
				pirates = pirates - 1

				if Engine.rand:Number(1) <= risk then
					local shipdef = shipdefs[Engine.rand:Integer(1,#shipdefs)]
					local default_drive = Equipment.hyperspace['hyperdrive_'..tostring(shipdef.hyperdriveClass)]

					local max_laser_size = shipdef.capacity - default_drive.capabilities.mass
					local laserdefs = utils.build_array(utils.filter(
						function (k,l) return l:IsValidSlot('laser_front') and l.capabilities.mass <= max_laser_size and l.l10n_key:find("PULSECANNON") end,
						pairs(Equipment.laser)
					))
					local laserdef = laserdefs[Engine.rand:Integer(1,#laserdefs)]

					pirate = Space.SpawnShipNear(shipdef.id, Game.player, 50, 100)
					pirate:SetLabel(Ship.MakeRandomLabel())
					pirate:AddEquip(default_drive)
					pirate:AddEquip(laserdef)
					pirate:AIKill(Game.player)
					table.insert(pirate_ships, pirate)
				end
			end

			if pirate then
				local pirate_greeting = string.interp(l["PIRATE_TAUNTS_" .. Engine.rand:Integer(1, getNumberOf("PIRATE_TAUNTS"))], {
					client = mission.client.name, location = mission.location:GetSystemBody().name,})
				Comms.ImportantMessage(pirate_greeting, pirate.label)
				pirate_gripes_time = Game.time
				if mission.wholesaler or Engine.rand:Number(0, 1) >= 0.75 then
					local escort
					escort = Space.SpawnShipNear("kanara", Game.player, 50, 100) -- Local wholesaler or random police ship
					escort:AddEquip(Equipment.laser.pulsecannon_1mw)
					escort:AddEquip(Equipment.misc.shield_generator)
					escort:SetLabel(Ship.MakeRandomLabel())
					escort:AIKill(pirate)
					table.insert(escort_ships, escort)
					Comms.ImportantMessage(l["ESCORT_CHATTER_" .. Engine.rand:Integer(1, getNumberOf("ESCORT_CHATTER"))], escort.label)
					escort_chatter_time = Game.time
					escort_switch_target = Game.time + Engine.rand:Integer(90, 120)
					pirate_switch_target = Game.time + Engine.rand:Integer(90, 120)
				end
			end
		end

		if mission.status == "ACTIVE" and Game.time > mission.due then
			mission.status = 'FAILED'
		end
	end
end

local isEscortShip = function (ship)
	if #escort_ships ~= 0 then
		for i = 1, #escort_ships do
			if escort_ships[i] == ship then return true end
		end
	end
	return false
end

local isPirateShip = function (ship)
	if #pirate_ships ~= 0 then
		for i = 1, #pirate_ships do
			if pirate_ships[i] == ship then return true end
		end
	end
	return false
end

local onShipDestroyed = function (ship, attacker)
	if ship:IsPlayer() or attacker == nil then return end

	if isEscortShip(ship) then
		for i = 1, #escort_ships do
			if escort_ships[i] == ship then
				table.remove(escort_ships, i)
				if isPirateShip(attacker) then
					attacker:AIKill(Game.player)
				end
			end
		end
	elseif isPirateShip(ship) then
		for i = 1, #pirate_ships do
			if pirate_ships[i] == ship then
				table.remove(pirate_ships, i)
				if isEscortShip(attacker) then
					Comms.ImportantMessage(l.TARGET_DESTROYED, attacker.label)
					if #pirate_ships ~= 0 then
						attacker:AIKill(pirate_ships[1])
						escort_switch_target = Game.time + Engine.rand:Integer(90, 120)
					end
				end
			end
		end
	end
end

local onShipFiring = function (ship)
	if ship:IsPlayer() then return end

	if isEscortShip(ship) and Game.time >= escort_chatter_time then -- don't flood the control panel with messages
		Comms.ImportantMessage(l.GUNS_GUNS_GUNS, ship.label)
			escort_chatter_time = Game.time + Engine.rand:Integer(15, 45)
	end
end

local onShipHit = function (ship, attacker)
	if attacker == nil then return end
	if not attacker:isa('Ship') then return end

	if ship:IsPlayer() and #escort_ships ~= 0 and not isEscortShip(attacker) then
		if Game.time >= escort_switch_target then -- don't switch between targets too quick
			for i = 1, #escort_ships do
				escort_ships[i]:AIKill(attacker) -- all escort ships respond
			end
			Comms.ImportantMessage(l.I_AM_ON_MY_WAY, escort_ships[1].label)
			escort_switch_target = Game.time + Engine.rand:Integer(90, 120)
		end

	elseif isEscortShip(ship) then
		if attacker:IsPlayer() then
			Comms.ImportantMessage(l.CEASE_FIRE, ship.label)
		elseif isPirateShip(attacker) then
			if Game.time >= escort_switch_target then
				ship:AIKill(attacker)
				escort_switch_target = Game.time + Engine.rand:Integer(90, 120)
			end
			if Game.time >= escort_chatter_time then
				Comms.ImportantMessage(l.I_NEED_HELP, ship.label)
				escort_chatter_time = Game.time + Engine.rand:Integer(15, 45)
			end
		end

	elseif isPirateShip(ship) then
		if attacker:IsPlayer() then
			if Game.time >= pirate_gripes_time then -- don't flood the control panel with messages
				Comms.ImportantMessage(l["PIRATE_GRIPES_" .. Engine.rand:Integer(1, getNumberOf("PIRATE_GRIPES"))], ship.label)
				pirate_gripes_time = Game.time + Engine.rand:Integer(30, 90)
			end
		elseif isEscortShip(attacker) then
			if Game.time >= pirate_switch_target then
				ship:AIKill(attacker)
				pirate_switch_target = Game.time + Engine.rand:Integer(90, 120)
			end
		end
	end
end

local onLeaveSystem = function (ship)
	if ship:IsPlayer() then
		nearbysystems = nil
		pirate_ships = {}
		escort_ships = {}
	end
end

local unloadCargo = function (mission)
	local uc = Game.player:RemoveEquip(mission.cargotype, mission.cargo, "cargo")
	if uc ~= mission.cargo then -- this is ugly
		local list = Game.player:GetEquip("cargo")
		for _, i in pairs(list) do
			if i.l10n_key == mission.cargotype.l10n_key then
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
		if (mission.location == station.path and not mission.pickup) or (mission.domicile == station.path and mission.pickup and mission.cargo_picked_up) then
			local reputation = mission.localdelivery and 1 or 1.5
			local oldReputation = Character.persistent.player.reputation
			local cargo = unloadCargo(mission)

			if Game.time <= mission.due and cargo == mission.cargo then
				local n = getNumberOf("SUCCESSMSG_" .. mission.branch)
				if n >= 1 then
					Comms.ImportantMessage(l["SUCCESSMSG_" .. mission.branch .. "_" .. Engine.rand:Integer(1, n)], mission.client.name)
				else
					Comms.ImportantMessage(l["SUCCESSMSG_" .. Engine.rand:Integer(1, getNumberOf("SUCCESSMSG"))], mission.client.name)
				end
				Character.persistent.player.reputation = Character.persistent.player.reputation + reputation
				player:AddMoney(mission.reward)
			else
				if cargo < mission.cargo then
					Comms.ImportantMessage(l.WHAT_IS_THIS, mission.client.name)
					player:AddMoney((cargo - mission.cargo) * mission.cargotype.price) -- pay for the missing
					Comms.ImportantMessage(l.I_HAVE_DEBITED_YOUR_ACCOUNT, mission.client.name)
				else
					local n = getNumberOf("FAILUREMSG_" .. mission.branch)
					if n >= 1 then
						Comms.ImportantMessage(l["FAILUREMSG_" .. mission.branch .. "_" .. Engine.rand:Integer(1, n)], mission.client.name)
					else
						Comms.ImportantMessage(l["FAILUREMSG_" .. Engine.rand:Integer(1, getNumberOf("FAILUREMSG"))], mission.client.name)
					end
				end
				Character.persistent.player.reputation = Character.persistent.player.reputation - reputation
			end
			Event.Queue("onReputationChanged", oldReputation, Character.persistent.player.killcount,
				Character.persistent.player.reputation, Character.persistent.player.killcount)

			mission:Remove()
			missions[ref] = nil

		elseif mission.location == station.path and mission.pickup and not mission.cargo_picked_up then
			if Game.player.freeCapacity < mission.cargo then
				Comms.ImportantMessage(l.YOU_DO_NOT_HAVE_ENOUGH_EMPTY_CARGO_SPACE, mission.client.name)
			else
				Game.player:AddEquip(mission.cargotype, mission.cargo, "cargo")
				mission.cargo_picked_up = true
				Comms.ImportantMessage(l.WE_HAVE_LOADED_UP_THE_CARGO_ON_YOUR_SHIP, mission.client.name)
			end

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
	pirate_ships = {}
	escort_ships = {}
end

local onClick = function (mission)
	local danger
	local dist = Game.system and string.format("%.2f", Game.system:DistanceTo(mission.location)) or "???"
	if mission.localdelivery then
		danger = l.RISK_1
	else
		local branch
		if mission.wholesaler then branch = "WHOLESALER" else branch = mission.branch end
		danger = (l["RISK_" .. branch .. "_" .. math.floor(mission.risk * (getNumberOf("RISK_" .. branch) - 1)) + 1]
			or l["RISK_" .. math.floor(mission.risk * (getNumberOf("RISK") - 1)) + 1])
	end

	if not mission.pickup then return ui:Grid(2,1)
		:SetColumn(0,{ui:VBox(10):PackEnd({ui:MultiLineText((mission.introtext):interp({name = mission.client.name,
											cargoname = l[mission.cargotype.l10n_key],
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
													ui:Label(l.DISTANCE)
												})
											})
											:SetColumn(1, {
												ui:VBox():PackEnd({
													ui:Label(dist.." "..l.LY)
												})
											}),
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
		})})
		:SetColumn(1, {
			ui:VBox(10):PackEnd(InfoFace.New(mission.client))
		})
	else return ui:Grid(2,1)
		:SetColumn(0,{ui:VBox(10):PackEnd({ui:MultiLineText((mission.introtext):interp({name = mission.client.name,
											cargoname = l[mission.cargotype.l10n_key],
											starport = mission.location:GetSystemBody().name,
											system = mission.location:GetStarSystem().name,
											sectorx = mission.location.sectorX,
											sectory = mission.location.sectorY,
											sectorz = mission.location.sectorZ,
											dom_starport = mission.domicile:GetSystemBody().name,
											dom_system = mission.domicile:GetStarSystem().name,
											dom_sectorx = mission.domicile.sectorX,
											dom_sectory = mission.domicile.sectorY,
											dom_sectorz = mission.domicile.sectorZ,
											cash = Format.Money(mission.reward),
											dist = dist})
										),
										ui:Margin(10),
										ui:Grid(1,1)
											:SetColumn(0, {
												ui:VBox():PackEnd({
													ui:Label(l.PICKUP_FROM)
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
													ui:Label(l.DISTANCE)
												})
											})
											:SetColumn(1, {
												ui:VBox():PackEnd({
													ui:Label(dist.." "..l.LY)
												})
											}),
										ui:Margin(5),
										ui:Grid(1,1)
											:SetColumn(0, {
												ui:VBox():PackEnd({
													ui:Label(l.DELIVER_TO)
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
													ui:MultiLineText(mission.domicile:GetSystemBody().name)
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
													ui:MultiLineText(mission.domicile:GetStarSystem().name.." ("..mission.domicile.sectorX..","..mission.domicile.sectorY..","..mission.domicile.sectorZ..")")
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
													ui:Label((string.format("%.2f", Game.system:DistanceTo(mission.domicile)) or "???") .. " " .. l.LY)
												})
											}),
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
		})})
		:SetColumn(1, {
			ui:VBox(10):PackEnd(InfoFace.New(mission.client))
		})
	end
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
Event.Register("onShipFiring", onShipFiring)
Event.Register("onShipHit", onShipHit)
Event.Register("onShipDestroyed", onShipDestroyed)
Event.Register("onGameStart", onGameStart)
Event.Register("onGameEnd", onGameEnd)
Event.Register("onReputationChanged", onReputationChanged)

Mission.RegisterType('CargoRun',l.CARGORUN,onClick)

Serializer:Register("CargoRun", serialize, unserialize)
