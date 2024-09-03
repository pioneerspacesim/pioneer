-- Copyright © 2008-2024 Pioneer Developers. See AUTHORS.txt for details
-- Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

local Engine = require 'Engine'
local Lang = require 'Lang'
local Game = require 'Game'
local Space = require 'Space'
local Comms = require 'Comms'
local Event = require 'Event'
local Mission = require 'Mission'
local MissionUtils = require 'modules.MissionUtils'
local Format = require 'Format'
local Serializer = require 'Serializer'
local Character = require 'Character'
local Equipment = require 'Equipment'
local ShipDef = require 'ShipDef'
local Ship = require 'Ship'
local utils = require 'utils'

local l = Lang.GetResource("module-cargorun")
local l_ui_core = Lang.GetResource("ui-core")
local lc = Lang.GetResource 'core'

-- don't produce missions for further than this many light years away
local max_delivery_dist = 15
-- typical reward for delivery to a system max_delivery_dist away
local typical_reward = 35 * max_delivery_dist
-- typical reward for local delivery
local typical_reward_local = 35
-- Minimum amount paid for very close deliveries
local min_local_dist_pay = 10
-- max cargo per trip
local max_cargo = 10
local max_cargo_wholesaler = 100
-- factor for pickup missions
local pickup_factor = 2
-- the maximum price of the custom cargo
local max_price = 300

local custom_cargo = {}

-- Each branch should have a probability weight proportional to its size
local custom_cargo_weight_sum = 0

local ads = {}
local missions = {}

local setDefaultCustomCargo = function()
	custom_cargo_weight_sum = 0
	custom_cargo = require 'modules.CargoRun.CargoTypes'
	for branch,branch_array in pairs(custom_cargo) do
		custom_cargo[branch].weight = #branch_array.goods
		custom_cargo_weight_sum = custom_cargo_weight_sum + #branch_array.goods
	end	
end

local isQualifiedFor = function(reputation, ad)
	return
		reputation >= 12 or
		ad.localdelivery or
		(ad.risk < 0.1 and ad.urgency <= 0.1) or
		(ad.risk < 0.5 and ad.urgency <= 0.5 and reputation >= 4) or
		false
end

-- This function returns the number of flavours of the given string str
-- It is assumed that the first flavour has suffix '_1'
local getNumberOfFlavours = function (str)
	local num = 1

	while l:get(str .. "_" .. num) do
		num = num + 1
	end
	return num - 1
end

local getRiskMsg = function (mission)
	if mission.localdelivery then
		return l.RISK_1 -- very low risk -> no specific text to give no confusing answer
	else
		local branch
		if mission.wholesaler then branch = "WHOLESALER" else branch = mission.branch end
		return l:get("RISK_" .. branch .. "_" .. math.floor(mission.risk * (getNumberOfFlavours("RISK_" .. branch) - 1)) + 1)
			or l["RISK_" .. math.floor(mission.risk * (getNumberOfFlavours("RISK") - 1)) + 1]
	end
end

local onDelete = function (ref)
	ads[ref] = nil
end

local isEnabled = function (ref)
	return ads[ref] ~= nil and isQualifiedFor(Character.persistent.player.reputation, ads[ref])
end

local onChat

local postAdvert = function(station, ad)
	local desc = string.interp(l[ad.text], {
		system   = ad.location:GetStarSystem().name,
		cash     = Format.Money(ad.reward, false),
		starport = ad.location:GetSystemBody().name,
	})

	local ref = station:AddAdvert({
		title       = l[ad.title],
		description = desc,
		icon        = ad.urgency >=  0.8 and "haul_fast" or "haul",
		due         = ad.due,
		reward      = ad.reward,
		location    = ad.location,
		onChat      = onChat,
		onDelete    = onDelete,
		isEnabled   = isEnabled })
	ads[ref] = ad
end

onChat = function (form, ref, option)
	local ad = ads[ref]

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

	-- check if we already have a deal
	for _, m in pairs(missions) do
		if ad.client == m.client then
			form:SetMessage(l["DENY_AGAIN_" .. Engine.rand:Integer(1, getNumberOfFlavours("DENY_AGAIN"))])
			return
		end
	end

	form:AddNavButton(ad.location)

	if ad.negotiated_amount == nil then
		ad.negotiated_amount = ad.amount
		ad.negotiated_reward = ad.reward
	end

	if option == 0 then
		local introtext  = string.interp(l[ad.introtext], {
			name         = ad.client.name,
			cash         = Format.Money(ad.negotiated_reward,false),
			cargoname    = ad.cargotype:GetName(),
			starport     = ad.location:GetSystemBody().name,
			system       = ad.location:GetStarSystem().name,
			sectorx      = ad.location.sectorX,
			sectory      = ad.location.sectorY,
			sectorz      = ad.location.sectorZ,
			dom_starport = ad.domicile:GetSystemBody().name,
			dom_system   = ad.domicile:GetStarSystem().name,
			dom_sectorx  = ad.domicile.sectorX,
			dom_sectory  = ad.domicile.sectorY,
			dom_sectorz  = ad.domicile.sectorZ,
			dist         = string.format("%.2f", ad.dist),
		})
		form:SetMessage(introtext)

	elseif option == 1 then
		local n = getNumberOfFlavours("WHYSOMUCH_" .. ad.branch)
		if n >= 1 then
			form:SetMessage(string.interp(l["WHYSOMUCH_" .. ad.branch .. "_" .. Engine.rand:Integer(1, n)], { cargoname = ad.cargotype:GetName() }))
		elseif ad.urgency >= 0.8 then
			form:SetMessage(string.interp(l["WHYSOMUCH_URGENT_" .. Engine.rand:Integer( 1, getNumberOfFlavours("WHYSOMUCH_URGENT"))], { cargoname = ad.cargotype:GetName() }))
		else
			form:SetMessage(string.interp(l["WHYSOMUCH_" .. Engine.rand:Integer( 1, getNumberOfFlavours("WHYSOMUCH"))], { cargoname = ad.cargotype:GetName() }))
		end

	elseif option == 2 then
		local howmuch
		if ad.wholesaler then
			howmuch = string.interp(l["HOWMUCH_WHOLESALER_" .. Engine.rand:Integer(1, getNumberOfFlavours("HOWMUCH_WHOLESALER"))], {
				amount    = ad.amount,
				cargoname = ad.cargotype:GetName()})
		else
			if ad.amount > 1 then
				howmuch = string.interp(l["HOWMUCH_" .. Engine.rand:Integer(1,getNumberOfFlavours("HOWMUCH"))],
					{amount = ad.amount})
			else
				howmuch = string.interp(l["HOWMUCH_SINGULAR_" .. Engine.rand:Integer(1,getNumberOfFlavours("HOWMUCH_SINGULAR"))],
					{amount = ad.amount})
			end
		end
		form:SetMessage(howmuch)
		form:AddOption(l.IS_IT_NEGOTIABLE, 6)

	elseif option == 3 then
		---@type CargoManager
		local cargoMgr = Game.player:GetComponent('CargoManager')

		if not ad.pickup then
			if cargoMgr:GetFreeSpace() < ad.negotiated_amount then
				form:SetMessage(l.YOU_DO_NOT_HAVE_ENOUGH_EMPTY_CARGO_SPACE)
				form:RemoveNavButton()
				return
			elseif cargoMgr:GetTotalSpace() < ad.negotiated_amount then
				form:SetMessage(l.YOU_DO_NOT_HAVE_ENOUGH_CARGO_SPACE_ON_YOUR_SHIP)
				form:RemoveNavButton()
				return
			end
		end

		local cargo_picked_up
		if not ad.pickup then
			cargoMgr:AddCommodity(ad.cargotype, ad.negotiated_amount)
			cargo_picked_up = true
		else
			cargo_picked_up = false
		end

		form:RemoveAdvertOnClose()
		ads[ref] = nil
		local mission = {
			type            = "CargoRun",
			domicile        = ad.domicile,
			client          = ad.client,
			location        = ad.location,
			localdelivery   = ad.localdelivery,
			wholesaler      = ad.wholesaler,
			pickup          = ad.pickup,
			cargo_picked_up	= cargo_picked_up,
			introtext       = ad.introtext,
			risk            = ad.risk,
			reward          = ad.negotiated_reward,
			due             = ad.due,
			amount          = ad.negotiated_amount,
			branch          = ad.branch,
			cargotype       = ad.cargotype,
		}
		table.insert(missions,Mission.New(mission))

		if ad.amount ~= ad.negotiated_amount then
			-- recreate advert with the rest of the cargo
			ad.amount = ad.amount - ad.negotiated_amount
			ad.reward = ad.reward - ad.negotiated_reward
			ad.negotiated_amount = ad.amount
			ad.negotiated_reward = ad.reward

			postAdvert(ad.station, ad)
		end

		if ad.pickup then
			form:SetMessage(l["ACCEPTED_PICKUP_" .. Engine.rand:Integer(1, getNumberOfFlavours("ACCEPTED_PICKUP"))])
		else
			form:SetMessage(l["ACCEPTED_" .. Engine.rand:Integer(1, getNumberOfFlavours("ACCEPTED"))])
		end
		return

	elseif option == 4 then
		form:SetMessage(string.interp(l:get("URGENCY_" .. ad.branch .. "_" .. math.floor(ad.urgency * (getNumberOfFlavours("URGENCY_" .. ad.branch) - 1)) + 1)
			or l["URGENCY_" .. math.floor(ad.urgency * (getNumberOfFlavours("URGENCY") - 1)) + 1], { date = Format.Date(ad.due) }))

	elseif option == 5 then
		form:SetMessage(getRiskMsg(ad))

	elseif option == 6 then
		local howmuch
		if ad.amount == 1 then
			howmuch = string.interp(l["NEGOTIABLE_SINGULAR_" .. Engine.rand:Integer(1,getNumberOfFlavours("NEGOTIABLE_SINGULAR"))],
				{amount = ad.amount})
		elseif ad.negotiable then
			howmuch = string.interp(l["NEGOTIABLE_" .. Engine.rand:Integer(1, getNumberOfFlavours("NEGOTIABLE"))], {
				amount    = ad.amount,
				cargoname = ad.cargotype:GetName()})

			local amount = {}
			local button = 2

			amount[1] = ad.wholesaler and max_cargo or 1
			for i = 2, 5 do
				local val = math.ceil(i^2/25 * ad.amount)
				if val > amount[button-1] then
					amount[button] = val
					button = button + 1
				end
			end
			for i, val in pairs(amount) do
				form:AddOption(string.interp(l.OFFER, { amount = val, reward = Format.Money(math.ceil(ad.reward * val/ad.amount), false) }), 10+val)
			end
		else
			howmuch = string.interp(l["NEGOTIABLE_NO_" .. Engine.rand:Integer(1,getNumberOfFlavours("NEGOTIABLE_NO"))],
				{amount = ad.amount})
		end
		form:SetMessage(howmuch)

	elseif option > 10 then
		ad.negotiated_amount = option - 10
		ad.negotiated_reward = math.ceil(ad.reward * ad.negotiated_amount/ad.amount)
		form:SetMessage(string.interp(l.OK_WE_AGREE, { amount = ad.negotiated_amount, reward = Format.Money(ad.negotiated_reward, false) }))
	end

	if option == 2 or option == 6 then
		form:RemoveNavButton()
		form:AddOption(l.GO_BACK, 0)
	else
		form:AddOption(l.WHY_SO_MUCH_MONEY, 1)
		form:AddOption(l.HOW_MUCH_MASS, 2)
		form:AddOption(l.HOW_SOON_MUST_IT_BE_DELIVERED, 4)
		form:AddOption(l.WILL_I_BE_IN_ANY_DANGER, 5)
		form:AddOption(l.COULD_YOU_REPEAT_THE_ORIGINAL_REQUEST, 0)
		form:AddOption(l.OK_AGREED, 3)
	end
end

local findNearbyStations = function (station, minDist, maxDist)
	local nearbystations = {}
	for _,s in ipairs(Game.system:GetStationPaths()) do
		if s ~= station.path then
			local dist = station:DistanceTo(Space.GetBody(s.bodyIndex))
			if dist >= minDist and dist <= maxDist then
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
	error("Programming error from random cargo.")
end

local nearbysystems

local makeAdvert = function (station)
	local reward, due, location, nearbysystem, dist, nearbystations, amount
	local risk, wholesaler, pickup, branch, cargotype, missiontype, timeout
	local client = Character.New()
	local urgency = Engine.rand:Number(0, 1)
	local localdelivery = Engine.rand:Number(0, 1) > 0.5 and true or false
	local negotiable = Engine.rand:Number(0, 1) > 0.25 and true or false

	branch, cargotype = randomCargo()
	if localdelivery then
		nearbysystem = Game.system
		-- discard stations closer than 1000m and further than 20AU
		nearbystations = findNearbyStations(station, 1000, 1.4960e11 * 20)
		if #nearbystations == 0 then return nil end

		amount = Engine.rand:Integer(1, max_cargo)
		risk = 0 -- no risk for local delivery
		wholesaler = false -- no local wholesaler delivery
		pickup = Engine.rand:Number(0, 1) > 0.75 and true or false
		location, dist = table.unpack(nearbystations[Engine.rand:Integer(1,#nearbystations)])
		reward = typical_reward_local + math.max(math.sqrt(dist) / 15000, min_local_dist_pay) * (1.5+urgency) * (1+amount/max_cargo)
		due = (60*60*18 + MissionUtils.TravelTimeLocal(dist)) * 1.66 * (1.5 - urgency) * Engine.rand:Number(0.9, 1.1)
		timeout = due/2 -- timeout after half of the travel time

		if pickup then
			missiontype = "PICKUP_LOCAL"
		else
			missiontype = "LOCAL"
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
			amount = Engine.rand:Integer(max_cargo, max_cargo_wholesaler)
			missiontype = "WHOLESALER"
			pickup = false
		else
			amount = Engine.rand:Integer(1, max_cargo)
			pickup = Engine.rand:Number(0, 1) > 0.75 and true or false
			if pickup then
				missiontype = "PICKUP"
			else
				missiontype = branch
			end
		end

		risk = 0.75 * cargotype.price / max_price + Engine.rand:Number(0, 0.25) -- goods with price max_price have a risk of 0.75 to 1
		reward = (dist / max_delivery_dist) * typical_reward * (1+risk) * (1.5+urgency) * (1+amount/max_cargo_wholesaler) * Engine.rand:Number(0.8,1.2)
		due = MissionUtils.TravelTime(dist, location) * 1.66 * (1.5 - urgency) * Engine.rand:Number(0.9, 1.1)
		timeout = due/2 -- timeout after half of the travel time
	end

	if pickup then
		reward = reward * pickup_factor
		due = due * pickup_factor + Game.time
		timeout = timeout * pickup_factor + Game.time
	else
		due = due + Game.time
		timeout = timeout + Game.time
	end
	reward = utils.round(reward, 25)
	due = utils.round(due, 900)

	local n = getNumberOfFlavours("INTROTEXT_" .. missiontype)
	local introtext

	if n >= 1 then
		introtext = "INTROTEXT_" .. missiontype .. "_" .. Engine.rand:Integer(1, n)
	else
		introtext = "INTROTEXT_" .. Engine.rand:Integer(1, getNumberOfFlavours("INTROTEXT"))
	end

	local ad = {
		station       = station,
		domicile      = station.path,
		client        = client,
		location      = location,
		localdelivery = localdelivery,
		wholesaler    = wholesaler,
		pickup        = pickup,
		introtext     = introtext,
		dist          = dist,
		due           = due,
		timeout       = timeout,
		amount        = amount,
		negotiable    = negotiable,
		branch        = branch,
		cargotype     = cargotype,
		risk          = risk,
		urgency       = urgency,
		reward        = reward,
		faceseed      = Engine.rand:Integer(),
	}

	n = getNumberOfFlavours("ADTEXT_" .. missiontype)
	if n >= 1 then
		local rand = Engine.rand:Integer(1, n)
		ad.text = "ADTEXT_" .. missiontype .. "_" .. rand
		ad.title = "ADTITLE_" .. missiontype .. "_" .. rand
	else
		local rand = Engine.rand:Integer(1, getNumberOfFlavours("ADTEXT"))
		ad.text = "ADTEXT_" .. rand
		ad.title = "ADTITLE_" .. rand
	end

	postAdvert(station, ad)

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
		if ad.timeout < Game.time then
			ad.station:RemoveAdvert(ref)
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

			local shipdefs = utils.build_array(utils.filter(function (k,def) return def.tag == 'SHIP'
				and def.hyperdriveClass > 0 and (def.roles.pirate or def.roles.mercenary) end, pairs(ShipDef)))
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
				local pirate_greeting = string.interp(l["PIRATE_TAUNTS_" .. Engine.rand:Integer(1, getNumberOfFlavours("PIRATE_TAUNTS"))], {
					client = mission.client.name, location = mission.location:GetSystemBody().name,})
				Comms.ImportantMessage(pirate_greeting, pirate.label)
				pirate_gripes_time = Game.time

				if mission.wholesaler or Engine.rand:Number(0, 1) >= 0.75 then
					local shipdef = ShipDef[Game.system.faction.policeShip]
					local escort = Space.SpawnShipNear(shipdef.id, Game.player, 50, 100)
					escort:SetLabel(l_ui_core.POLICE)
					escort:AddEquip(Equipment.laser.pulsecannon_1mw)
					escort:AddEquip(Equipment.misc.shield_generator)
					escort:AIKill(pirate)
					table.insert(escort_ships, escort)

					Comms.ImportantMessage(l["ESCORT_CHATTER_" .. Engine.rand:Integer(1, getNumberOfFlavours("ESCORT_CHATTER"))], escort.label)
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
				Comms.ImportantMessage(l["PIRATE_GRIPES_" .. Engine.rand:Integer(1, getNumberOfFlavours("PIRATE_GRIPES"))], ship.label)
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

local onShipDocked = function (player, station)
	if not player:IsPlayer() then return end

	-- First drop off cargo (if any such missions)
	for ref,mission in pairs(missions) do
		local readyToComplete = (mission.location == station.path and not mission.pickup) or
			(mission.domicile == station.path and mission.pickup and mission.cargo_picked_up)

		if readyToComplete then

			local reputation = mission.localdelivery and 1 or 1.5
			local oldReputation = Character.persistent.player.reputation

			---@type CargoManager
			local cargoMgr = Game.player:GetComponent('CargoManager')
			local amount = cargoMgr:RemoveCommodity(mission.cargotype, mission.amount)

			if Game.time <= mission.due and amount == mission.amount then
				local n = getNumberOfFlavours("SUCCESSMSG_" .. mission.branch)
				if n >= 1 then
					Comms.ImportantMessage(l["SUCCESSMSG_" .. mission.branch .. "_" .. Engine.rand:Integer(1, n)], mission.client.name)
				else
					Comms.ImportantMessage(l["SUCCESSMSG_" .. Engine.rand:Integer(1, getNumberOfFlavours("SUCCESSMSG"))], mission.client.name)
				end

				Character.persistent.player.reputation = Character.persistent.player.reputation + reputation
				player:AddMoney(mission.reward)
			else
				if amount < mission.amount then
					Comms.ImportantMessage(l.WHAT_IS_THIS, mission.client.name)
					player:AddMoney((amount - mission.amount) * mission.cargotype.price) -- pay for the missing
					Comms.ImportantMessage(l.I_HAVE_DEBITED_YOUR_ACCOUNT, mission.client.name)
				else
					local n = getNumberOfFlavours("FAILUREMSG_" .. mission.branch)
					if n >= 1 then
						Comms.ImportantMessage(l["FAILUREMSG_" .. mission.branch .. "_" .. Engine.rand:Integer(1, n)], mission.client.name)
					else
						Comms.ImportantMessage(l["FAILUREMSG_" .. Engine.rand:Integer(1, getNumberOfFlavours("FAILUREMSG"))], mission.client.name)
					end
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

	-- Now we have space pick up cargo as well
	for ref,mission in pairs(missions) do
		local readyToPickup = mission.location == station.path and mission.pickup and not mission.cargo_picked_up

		if readyToPickup then
			if Game.time < mission.due then

				---@type CargoManager
				local cargoMgr = Game.player:GetComponent('CargoManager')

				if cargoMgr:GetFreeSpace() < mission.amount then
					Comms.ImportantMessage(l.YOU_DO_NOT_HAVE_ENOUGH_EMPTY_CARGO_SPACE, mission.client.name)
				else
					cargoMgr:AddCommodity(mission.cargotype, mission.amount);
					mission.cargo_picked_up = true
					Comms.ImportantMessage(l.WE_HAVE_LOADED_UP_THE_CARGO_ON_YOUR_SHIP, mission.client.name)
				end

			else

				local oldReputation = Character.persistent.player.reputation
				Character.persistent.player.reputation = Character.persistent.player.reputation - (mission.localdelivery and 1 or 1.5)

				Comms.ImportantMessage(l.TOO_LATE_TO_PICK_UP, mission.client.name)
				Event.Queue("onReputationChanged", oldReputation, Character.persistent.player.killcount,
					Character.persistent.player.reputation, Character.persistent.player.killcount)

				mission:Remove()
				missions[ref] = nil

			end
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
	if loaded_data and loaded_data.ads then
		ads = {}
		missions = {}
		custom_cargo = {}
		custom_cargo_weight_sum = 0

		for k,ad in pairs(loaded_data.ads) do
			postAdvert(ad.station, ad)
		end
		missions = loaded_data.missions
		custom_cargo = loaded_data.custom_cargo
		custom_cargo_weight_sum = loaded_data.custom_cargo_weight_sum
		loaded_data = nil
	else
		setDefaultCustomCargo()
	end
end

local onGameEnd = function ()
	nearbysystems = nil
	pirate_ships = {}
	escort_ships = {}

	ads = {}
	missions = {}
	custom_cargo = {}
	custom_cargo_weight_sum = 0
end

local buildMissionDescription = function(mission)
	local ui = require 'pigui'
	local desc = {}
	local dist = Game.system and string.format("%.2f", Game.system:DistanceTo(mission.location)) or "???"
	local danger = getRiskMsg(mission)

	desc.description = l[mission.introtext]:interp({
		name = mission.client.name,
		cargoname = mission.cargotype:GetName(),
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
		cash = ui.Format.Money(mission.reward,false),
		dist = dist
	})

	desc.location = mission.location
	desc.client = mission.client

	if not mission.pickup then
		desc.details = {
			l.DELIVER_TO,
			{ l.SPACEPORT,	mission.location:GetSystemBody().name },
			{ l.SYSTEM,		ui.Format.SystemPath(mission.location) },
			{ l.DISTANCE,	dist.." "..lc.UNIT_LY },
			false,
			{ l.DEADLINE,	ui.Format.Date(mission.due) },
			{ l.CARGO,		mission.cargotype:GetName() },
			{ l.AMOUNT,		mission.amount.."t " },
			{ l.DANGER,		danger },
		}
	else
		local domicileDist = Game.system and string.format("%.2f", Game.system:DistanceTo(mission.domicile)) or "???"
		local is_cargo_loaded = mission.cargo_picked_up and l.CARGO_IS_LOADED or l.CARGO_IS_NOT_LOADED

		desc.details = {
			l.PICKUP_FROM,
			{ l.SPACEPORT,	mission.location:GetSystemBody().name },
			{ l.SYSTEM,		ui.Format.SystemPath(mission.location) },
			{ l.DISTANCE,	dist.." "..lc.UNIT_LY },
			l.DELIVER_TO,
			{ l.SPACEPORT,	mission.domicile:GetSystemBody().name },
			{ l.SYSTEM,		ui.Format.SystemPath(mission.domicile) },
			{ l.DISTANCE,	domicileDist.. " " .. lc.UNIT_LY },
			false,
			{ l.DEADLINE,	ui.Format.Date(mission.due) },
			{ l.CARGO,		mission.cargotype:GetName() },
			{ l.AMOUNT,		mission.amount.."t "..is_cargo_loaded },
			{ l.DANGER,		danger },
		}

		desc.returnLocation = mission.domicile
	end

	return desc
end

local serialize = function ()
	return { ads = ads, missions = missions, custom_cargo = custom_cargo, custom_cargo_weight_sum = custom_cargo_weight_sum }
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

Mission.RegisterType('CargoRun',l.CARGORUN, buildMissionDescription)

Serializer:Register("CargoRun", serialize, unserialize)
