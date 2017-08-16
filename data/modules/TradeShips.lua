-- Copyright © 2008-2017 Pioneer Developers. See AUTHORS.txt for details
-- Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

local Engine = import("Engine")
local Game = import("Game")
local Space = import("Space")
local Comms = import("Comms")
local Timer = import("Timer")
local Event = import("Event")
local Serializer = import("Serializer")
local ShipDef = import("ShipDef")
local Ship = import("Ship")
local utils = import("utils")
local e = import ("Equipment")

--[[
	trade_ships
		interval - is minimum amount of time between hyperspace arrivals,
			stored here as it needs to be saved; number as seconds, updated by
			spawnInitialShips
		ship - object returned from Space:SpawnShip*
			ship_name - of this ship type; string
			ATMOSHIELD - flag indicating whether the ship has at atmospheric shield: boolean
			starport - at which this ship intends to dock; SpaceStation object
			dest_time - arrival time from hyperspace; number as Game.time
			dest_path - for hyperspace; SystemPath object, may have body index
			from_path - for hyperspace; SystemPath object
			delay - indicates waiting for a Timer to give next action; number
				as Game.time
			status - of this ship; string, one of:
				hyperspace - yet to arrive or has departed
				inbound - in system and given AIDockWith order
				docked - currently docked or un/docking
				outbound - heading away from starport before hyperspacing
				fleeing - has been attacked and is trying to get away
					(currently still just following whatever AI order it had)
				cowering - docked after having been attacked, waiting for
					attacker to go away
				orbit - was unable to dock, heading to or waiting in orbit
			cargo - table of cargo types and amounts currently carried;
				key: Constants.EquipType string, value: number
			attacker - what this was last attacked by; Body object
			chance - used to determine what action to take when attacked; number
			last_flee - when last action was taken, number as Game.time
			no_jump - whether this has tried to hyperspace away so it only
				tries once; bool

	system_updated - indicates whether the following tables have been updated
		for the current system; bool, see onEnterSystem, onLeaveSystem, and
		onGameStart

	from_paths - paths of systems around the current system, used to get a
		from_system for ships spawned in hyperspace; indexed array of
		SystemPath objects, updated by spawnInitialShips

	starports - in the current system; indexed array of SpaceStation objects,
		updated by spawnInitialShips

	vacuum_starports - in the current system; indexed array of SpaceStation objects that can be
		approached without atmospheric shields,	updated by spawnInitialShips

	imports, exports - in the current system, indexed array of
		equipment objects (from the 'Equipment' module), updated by spawnInitialShips
--]]
local trade_ships, system_updated, from_paths, starports, vacuum_starports, imports, exports

local addFuel = function (ship)
	local drive = ship:GetEquip('engine', 1)

	-- a drive must be installed
	if not drive then
		print(trade_ships[ship]['ship_name']..' has no drive!')
		return nil
	end

	-- the last character of the fitted drive is the class
	-- the fuel needed for max range is the square of the drive class
	local count = drive.capabilities.hyperclass ^ 2

	-- account for fuel it already has
	count = count - ship:CountEquip(e.cargo.hydrogen)

	local added = ship:AddEquip(e.cargo.hydrogen, count)

	return added
end

local addShipEquip = function (ship)
	local trader = trade_ships[ship]
	local ship_type = ShipDef[trader.ship_name]

	-- add standard equipment
	ship:AddEquip(e.hyperspace['hyperdrive_'..tostring(ship_type.hyperdriveClass)])
	if ShipDef[ship.shipId].equipSlotCapacity.atmo_shield > 0 then
		ship:AddEquip(e.misc.atmospheric_shielding)
		trader.ATMOSHIELD = true -- flag this to save function calls later
	else
		-- This ship cannot safely land on a planet with an atmosphere.
		trader.ATMOSHIELD = false
	end
	ship:AddEquip(e.misc.radar)
	ship:AddEquip(e.misc.autopilot)
	ship:AddEquip(e.misc.cargo_life_support)

	-- add defensive equipment based on lawlessness, luck and size
	local lawlessness = Game.system.lawlessness
	local size_factor = ship.freeCapacity ^ 2 / 2000000

	if Engine.rand:Number(1) - 0.1 < lawlessness then
		local num = math.floor(math.sqrt(ship.freeCapacity / 50)) -
					 ship:CountEquip(e.misc.shield_generator)
		if num > 0 then ship:AddEquip(e.misc.shield_generator, num) end
		if ship_type.equipSlotCapacity.energy_booster > 0 and
		Engine.rand:Number(1) + 0.5 - size_factor < lawlessness then
			ship:AddEquip(e.misc.shield_energy_booster)
		end
	end

	-- we can't use these yet
	if ship_type.equipSlotCapacity.ecm > 0 then
		if Engine.rand:Number(1) + 0.2 < lawlessness then
			ship:AddEquip(e.misc.ecm_advanced)
		elseif Engine.rand:Number(1) < lawlessness then
			ship:AddEquip(e.misc.ecm_basic)
		end
	end

	-- this should be rare
	if ship_type.equipSlotCapacity.hull_autorepair > 0 and
	Engine.rand:Number(1) + 0.75 - size_factor < lawlessness then
		ship:AddEquip(e.misc.hull_autorepair)
	end
end

local addShipCargo = function (ship, direction)
	local total = 0
	local empty_space = math.min(ship.freeCapacity, ship:GetEquipFree("cargo"))
	local size_factor = empty_space / 20
	local ship_cargo = {}

	if direction == 'import' and #imports == 1 then
		total = ship:AddEquip(imports[1], empty_space)
		ship_cargo[imports[1]] = total
	elseif direction == 'export' and #exports == 1 then
		total = ship:AddEquip(exports[1], empty_space)
		ship_cargo[exports[1]] = total
	elseif (direction == 'import' and #imports > 1) or
			(direction == 'export' and #exports > 1) then

		-- happens if there was very little space left to begin with (eg small
		-- ship with lots of equipment). if we let it through then we end up
		-- trying to add 0 cargo forever
		if size_factor < 1 then
			trade_ships[ship]['cargo'] = ship_cargo
			return 0
		end

		while total < empty_space do
			local cargo_type

			-- get random for direction
			if direction == 'import' then
				cargo_type = imports[Engine.rand:Integer(1, #imports)]
			else
				cargo_type = exports[Engine.rand:Integer(1, #exports)]
			end

			-- amount based on price and size of ship
			local num = math.abs(Game.system:GetCommodityBasePriceAlterations(cargo_type)) * size_factor
			num = Engine.rand:Integer(num, num * 2)

			local added = ship:AddEquip(cargo_type, num)
			if ship_cargo[cargo_type] == nil then
				ship_cargo[cargo_type] = added
			else
				ship_cargo[cargo_type] = ship_cargo[cargo_type] + added
			end
			total = total + added
		end
	end
	-- if the table for direction was empty then cargo is empty and total is 0

	trade_ships[ship]['cargo'] = ship_cargo
	return total
end

local doUndock
doUndock = function (ship)
	-- the player may have left the system or the ship may have already undocked
	if ship:exists() and ship:GetDockedWith() then
		local trader = trade_ships[ship]
		if not ship:Undock() then
			-- unable to undock, try again in ten minutes
			trader['delay'] = Game.time + 600
			Timer:CallAt(trader.delay, function () doUndock(ship) end)
		else
			trader['delay'] = nil
		end
	end
end

local doOrbit = function (ship)
	local trader = trade_ships[ship]
	local sbody = trader.starport.path:GetSystemBody()
	local body = Space.GetBody(sbody.parent.index)
	ship:AIEnterLowOrbit(body)
	trader['status'] = 'orbit'
	print(ship.label..' ordering orbit of '..body.label)
end

local getNearestStarport = function (ship, current)
	if #starports == 0 then return nil end

	local trader = trade_ships[ship]

	-- Find the nearest starport that we can land at (other than current)
	local starport, distance
	for i = 1, #starports do
		local next_starport = starports[i]
		if next_starport ~= current then
			local next_distance = ship:DistanceTo(next_starport)
			local next_canland = (trader.ATMOSHIELD or
				(next_starport.type == 'STARPORT_ORBITAL') or
				(not next_starport.path:GetSystemBody().parent.hasAtmosphere))

			if next_canland and ((starport == nil) or (next_distance < distance)) then
				starport, distance = next_starport, next_distance
			end
		end
	end
	return starport or current
end

local getSystem = function (ship)
	local max_range = ship.hyperspaceRange;
	if max_range > 30 then
		max_range = 30
	end
	local min_range = max_range / 2;
	if min_range < 7.5 then
		min_range = 7.5
	end
	local systems_in_range = Game.system:GetNearbySystems(min_range)
	if #systems_in_range == 0 then
		systems_in_range = Game.system:GetNearbySystems(max_range)
	end
	if #systems_in_range == 0 then return nil end
	if #systems_in_range == 1 then
		return systems_in_range[1].path
	end

	local target_system = nil
	local best_prices = 0

	-- find best system for cargo
	for _, next_system in ipairs(systems_in_range) do
		if #next_system:GetStationPaths() > 0 then
			local next_prices = 0
			for cargo, count in pairs(trade_ships[ship]['cargo']) do
				next_prices = next_prices + (next_system:GetCommodityBasePriceAlterations(cargo) * count)
			end
			if next_prices > best_prices then
				target_system, best_prices = next_system, next_prices
			end
		end
	end

	if target_system == nil then
		-- pick a random system as fallback
		target_system = systems_in_range[Engine.rand:Integer(1, #systems_in_range)]

		-- get closer systems
		local systems_half_range = Game.system:GetNearbySystems(min_range)

		if #systems_half_range > 1 then
			target_system = systems_half_range[Engine.rand:Integer(1, #systems_half_range)]
		end
	end

	-- pick a random starport, if there are any, so the game can simulate
	-- travel to it if player arrives after (see Space::DoHyperspaceTo)
	local target_starport_paths = target_system:GetStationPaths()
	if #target_starport_paths > 0 then
		return target_starport_paths[Engine.rand:Integer(1, #target_starport_paths)]
	end

	return target_system.path
end

local jumpToSystem = function (ship, target_path)
	if target_path == nil then return nil end

	local status, fuel, duration = ship:HyperjumpTo(target_path)

	if status ~= 'OK' then
		print(trade_ships[ship]['ship_name']..' jump status is not OK')
		return status
	end

	-- update table for ship
	trade_ships[ship]['status'] = 'hyperspace'
	trade_ships[ship]['starport'] = nil
	trade_ships[ship]['dest_time'] = Game.time + duration
	trade_ships[ship]['dest_path'] = target_path
	trade_ships[ship]['from_path'] = Game.system.path
	return status
end

local getSystemAndJump = function (ship)
	return jumpToSystem(ship, getSystem(ship))
end

local getAcceptableShips = function ()
	-- only accept ships with enough capacity that are capable of landing in atmospheres
	local filter_function
	if #vacuum_starports == 0 then
		filter_function = function(k,def)
			-- XXX should limit to ships large enough to carry significant
			--     cargo, but we don't have enough ships yet
			return def.tag == 'SHIP' and def.hyperdriveClass > 0 and def.equipSlotCapacity.atmo_shield > 0
		end
	else
		filter_function = function(k,def)
			-- XXX should limit to ships large enough to carry significant
			--     cargo, but we don't have enough ships yet
			return def.tag == 'SHIP' and def.hyperdriveClass > 0
		end
	end
	return utils.build_array(
		utils.map(function (k,def)
			return k,def.id
		end,
		utils.filter(filter_function,
		pairs(ShipDef)
	)))
end

local spawnInitialShips = function (game_start)
	-- check if the current system can be traded in
	starports = Space.GetBodies(function (body) return body.superType == 'STARPORT' end)
	if #starports == 0 then return nil end
	vacuum_starports = Space.GetBodies(function (body)
		return body.superType == 'STARPORT' and (body.type == 'STARPORT_ORBITAL' or (not body.path:GetSystemBody().parent.hasAtmosphere))
	end)
	local population = Game.system.population
	if population == 0 then return nil end
	local ship_names = getAcceptableShips()
	if #ship_names == 0 then return nil end

	-- get a measure of the market size and build lists of imports and exports
	local import_score, export_score = 0, 0
	imports, exports = {}, {}
	for key, equip in pairs(e.cargo) do
		local v = Game.system:GetCommodityBasePriceAlterations(equip)
		if key ~= "rubbish" and key ~= "radioactives" and Game.system:IsCommodityLegal(equip) then
			-- values from SystemInfoView::UpdateEconomyTab
			if		v > 10	then
				import_score = import_score + 2
			elseif	v > 2	then
				import_score = import_score + 1
				table.insert(imports, equip)
			elseif	v < -10	then
				export_score = export_score + 2
			elseif	v < -2	then
				export_score = export_score + 1
				table.insert(exports, equip)
			end
		end
	end
	-- if there is no market then there is no trade
	if #imports == 0 or #exports == 0 then return nil end

	-- determine how many trade ships to spawn
	local lawlessness = Game.system.lawlessness
	-- start with three ships per two billion population
	local num_trade_ships = population * 1.5
	-- add the average of import_score and export_score
	num_trade_ships = num_trade_ships + (import_score + export_score) / 2
	-- reduce based on lawlessness
	num_trade_ships = num_trade_ships * (1 - lawlessness)
	-- vary by up to twice as many with a bell curve probability
	num_trade_ships = num_trade_ships * (Engine.rand:Number(0.25, 1) + Engine.rand:Number(0.25, 1))
	-- compute distance and interval between ships
	-- the base number of AU between ships spawned in space
	local range = (9 / (num_trade_ships * 0.75))
	if game_start then
		range = range * 1.5
	end
	-- the base number of seconds between ships spawned in hyperspace
	trade_ships['interval'] = (864000 / (num_trade_ships / 4))
	-- get nearby system paths for hyperspace spawns to come from
	local from_systems, dist = {}, 10
	while #from_systems < 10 do
		from_systems = Game.system:GetNearbySystems(dist)
		dist = dist + 5
	end
	from_paths = {}
	for _, system in ipairs(from_systems) do
		table.insert(from_paths, system.path)
	end

	-- spawn the initial trade ships
	for i = 0, num_trade_ships do
		-- get the name of a ship, for example 'imperial_courier'
		local ship_name = ship_names[Engine.rand:Integer(1, #ship_names)]
		local ship = nil

		if game_start and i < num_trade_ships / 4 then
			-- spawn the first quarter in port if at game start
			local starport = starports[Engine.rand:Integer(1, #starports)]

			ship = Space.SpawnShipDocked(ship_name, starport)
			if ship ~= nil then
				ship:SetLabel(Ship.MakeRandomLabel())
				trade_ships[ship] = {
					status		= 'docked',
					starport	= starport,
					ship_name	= ship_name,
				}
				addShipEquip(ship)
			else
				-- the starport must have been full
				ship = Space.SpawnShipNear(ship_name, starport, 10000000, 149598000) -- 10mkm - 1AU
				ship:SetLabel(Ship.MakeRandomLabel())
				trade_ships[ship] = {
					status		= 'inbound',
					starport	= starport,
					ship_name	= ship_name,
				}
				addShipEquip(ship)
			end
		elseif i < num_trade_ships * 0.75 then
			-- spawn the first three quarters in space, or middle half if game start
			local min_dist = range * i + 1
			if game_start then
				min_dist = min_dist - (range * (num_trade_ships / 4))
			end

			ship = Space.SpawnShip(ship_name, min_dist, min_dist + range)
			ship:SetLabel(Ship.MakeRandomLabel())
			trade_ships[ship] = {
				status		= 'inbound',
				ship_name	= ship_name,
			}
			-- Add ship equipment right now, because...
			addShipEquip(ship)
			-- ...this next call needs to see if there's an atmospheric shield.
			trade_ships[ship].starport	= getNearestStarport(ship)
		else
			-- spawn the last quarter in hyperspace
			local min_time = trade_ships.interval * (i - num_trade_ships * 0.75)
			local max_time = min_time + trade_ships.interval
			local dest_time = Game.time + Engine.rand:Integer(min_time, max_time)
			local from = from_paths[Engine.rand:Integer(1, #from_paths)]

			ship = Space.SpawnShip(ship_name, 9, 11, {from, dest_time})
			ship:SetLabel(Ship.MakeRandomLabel())
			trade_ships[ship] = {
				status		= 'hyperspace',
				dest_time	= dest_time,
				dest_path	= Game.system.path,
				from_path	= from,
				ship_name	= ship_name,
			}
			addShipEquip(ship)
		end
		local trader = trade_ships[ship]

		-- add cargo
		local fuel_added = addFuel(ship)
		if trader.status == 'docked' then
			local delay = fuel_added + addShipCargo(ship, 'export')
			-- have ship wait 30-45 seconds per unit of cargo
			if delay > 0 then
				trader['delay'] = Game.time + (delay * Engine.rand:Number(30, 45))
			else
				trader['delay'] = Game.time + Engine.rand:Number(600, 3600)
			end
			Timer:CallAt(trader.delay, function () doUndock(ship) end)
		else
			addShipCargo(ship, 'import')
			-- remove fuel used to get here
			if fuel_added and fuel_added > 0 then
				ship:RemoveEquip(e.cargo.hydrogen, Engine.rand:Integer(1, fuel_added))
			end
			if trader.status == 'inbound' then
				ship:AIDockWith(trader.starport)
			end
		end
	end

	return num_trade_ships
end

local spawnReplacement = function ()
	-- spawn new ship in hyperspace
	if #starports > 0 and Game.system.population > 0 and #imports > 0 and #exports > 0 then
		local ship_names = getAcceptableShips()
		local ship_name = ship_names[Engine.rand:Integer(1, #ship_names)]

		local dest_time = Game.time + Engine.rand:Number(trade_ships.interval, trade_ships.interval * 2)
		local from = from_paths[Engine.rand:Integer(1, #from_paths)]

		local ship = Space.SpawnShip(ship_name, 9, 11, {from, dest_time})
		ship:SetLabel(Ship.MakeRandomLabel())
		trade_ships[ship] = {
			status		= 'hyperspace',
			dest_time	= dest_time,
			dest_path	= Game.system.path,
			from_path	= from,
			ship_name	= ship_name,
		}

		addShipEquip(ship)
		local fuel_added = addFuel(ship)
		addShipCargo(ship, 'import')
		if fuel_added and fuel_added > 0 then
			ship:RemoveEquip(e.cargo.hydrogen, Engine.rand:Integer(1, fuel_added))
		end
	end
end

local updateTradeShipsTable = function ()
	local total, removed = 0, 0
	for ship, trader in pairs(trade_ships) do
		total = total + 1
		if trader.status == 'hyperspace' then
			-- remove ships not coming here
			if not trader.dest_path:IsSameSystem(Game.system.path) then
				trade_ships[ship] = nil
				removed = removed + 1
			end
		else
			-- remove ships that are not in hyperspace
			trade_ships[ship] = nil
			removed = removed + 1
		end
	end
	print('updateTSTable:total:'..total..',removed:'..removed)
end

local cleanTradeShipsTable = function ()
	local total, hyperspace, removed = 0, 0, 0
	for ship, trader in pairs(trade_ships) do
		if ship ~= 'interval' then
			total = total + 1
			if trader.status == 'hyperspace' then
				hyperspace = hyperspace + 1
				-- remove well past due ships as the player can not catch them
				if trader.dest_time + 86400 < Game.time then
					trade_ships[ship] = nil
					removed = removed + 1
				end
			end
		end
	end
	print('cleanTSTable:total:'..total..',active:'..total - hyperspace..',removed:'..removed)
end

local onEnterSystem = function (ship)
	-- if the player is following a ship through hyperspace that ship may enter first
	-- so update the system when the first ship enters (see Space::DoHyperspaceTo)
	if not system_updated then
		updateTradeShipsTable()
		spawnInitialShips(false)
		system_updated = true
	end

	if trade_ships[ship] ~= nil then
		local trader = trade_ships[ship]
		print(ship.label..' '..trader.ship_name..' entered '..Game.system.name..' from '..trader.from_path:GetStarSystem().name)

		local starport = getNearestStarport(ship)
		if starport then
			ship:AIDockWith(starport)
			trade_ships[ship]['starport'] = starport
			trade_ships[ship]['status'] = 'inbound'
		else
			-- starport == nil happens if player has followed ship to empty system, or
			-- no suitable port found (e.g. all stations atmospheric for ship without atmoshield)
			getSystemAndJump(ship)
			-- if we couldn't reach any systems wait for player to attack
		end
	end
end
Event.Register("onEnterSystem", onEnterSystem)

local onLeaveSystem = function (ship)
	if ship:IsPlayer() then
		-- the next onEnterSystem will be in a new system
		system_updated = false
		trade_ships['interval'] = nil

		local total, removed = 0, 0
		for t_ship, trader in pairs(trade_ships) do
			total = total + 1
			if trader.status == 'hyperspace' then
				if trader.dest_path:IsSameSystem(Game.system.path) then
					-- remove ships that are in hyperspace to here
					trade_ships[t_ship] = nil
					removed = removed + 1
				end
			else
				-- remove all ships that are not in hyperspace
				trade_ships[t_ship] = nil
				removed = removed + 1
			end
		end
		print('onLeaveSystem:total:'..total..',removed:'..removed)
	elseif trade_ships[ship] ~= nil then
		local system = trade_ships[ship]['dest_path']:GetStarSystem()
		print(ship.label..' left '..Game.system.name..' for '..system.name)
		cleanTradeShipsTable()
		spawnReplacement()
	end
end
Event.Register("onLeaveSystem", onLeaveSystem)

local onFrameChanged = function (ship)
	if not ship:isa("Ship") or trade_ships[ship] == nil then return end
	local trader = trade_ships[ship]

	if trader.status == 'outbound' then
		-- the cloud inherits the ship velocity and vector
		ship:CancelAI()
		if getSystemAndJump(ship) ~= 'OK' then
			ship:AIDockWith(trader.starport)
			trader['status'] = 'inbound'
		end
	end
end
Event.Register("onFrameChanged", onFrameChanged)

local onShipDocked = function (ship, starport)
	if trade_ships[ship] == nil then return end
	local trader = trade_ships[ship]

	print(ship.label..' docked with '..starport.label..' ship:'..trader.ship_name)

	if trader.status == 'fleeing' then
		trader['status'] = 'cowering'
	else
		trader['status'] = 'docked'
	end
	if trader.chance then
		trader['chance'] = trader.chance / 2
		trader['last_flee'], trader['no_jump'] = nil, nil
	end

	-- 'sell' trade cargo
	local delay = 0
	for cargo, _ in pairs(trader.cargo) do
		delay = delay + ship:RemoveEquip(cargo, 1000000)
	end

	local damage = ShipDef[trader.ship_name].hullMass -
					ship.hullMassLeft
	if damage > 0 then
		ship:SetHullPercent()
		addShipEquip(ship)
		damage = damage * 4
	end
	addFuel(ship)
	delay = delay + addShipCargo(ship, 'export')
	if damage > delay then delay = damage end

	-- delay undocking by 30-45 seconds for every unit of cargo transfered
	-- or 2-3 minutes for every unit of hull repaired
	if delay > 0 then
		trader['delay'] = Game.time + (delay * Engine.rand:Number(30, 45))
	else
		trader['delay'] = Game.time + Engine.rand:Number(600, 3600)
	end

	if trader.status == 'docked' then
		Timer:CallAt(trader.delay, function () doUndock(ship) end)
	end
end
Event.Register("onShipDocked", onShipDocked)

local onShipUndocked = function (ship, starport)
	if trade_ships[ship] == nil then return end

	-- fly to the limit of the starport frame
	ship:AIFlyTo(starport)

	trade_ships[ship]['status'] = 'outbound'
end
Event.Register("onShipUndocked", onShipUndocked)

local onAICompleted = function (ship, ai_error)
	if trade_ships[ship] == nil then return end
	local trader = trade_ships[ship]
	if ai_error ~= 'NONE' then
		print(ship.label..' AICompleted: Error: '..ai_error..' Status: '..trader.status) end

	if trader.status == 'outbound' then
		if getSystemAndJump(ship) ~= 'OK' then
			ship:AIDockWith(trader.starport)
			trader['status'] = 'inbound'
		end
	elseif trader.status == 'orbit' then
		if ai_error == 'NONE' then
			trader['delay'] = Game.time + 21600 -- 6 hours
			Timer:CallAt(trader.delay, function ()
				if ship:exists() and ship.flightState ~= 'HYPERSPACE' then
					trader['starport'] = getNearestStarport(ship, trader.starport)
					ship:AIDockWith(trader.starport)
					trader['status'] = 'inbound'
					trader['delay'] = nil
				end
			end)
		end
		-- XXX if ORBIT_IMPOSSIBLE asteroid? get parent of parent and attempt orbit?
	elseif trader.status == 'inbound' then
		if ai_error == 'REFUSED_PERM' then doOrbit(ship) end
	end
end
Event.Register("onAICompleted", onAICompleted)

local onShipLanded = function (ship, body)
	if trade_ships[ship] == nil then return end
	print(ship.label..' Landed: '..trade_ships[ship].starport.label)

	doOrbit(ship)
end
Event.Register("onShipLanded", onShipLanded)

local onShipAlertChanged = function (ship, alert)
	if trade_ships[ship] == nil then return end
	if alert == 'SHIP_FIRING' then
		print(ship.label..' alert changed to '..alert) end
	local trader = trade_ships[ship]
	if trader.attacker == nil then return end

	if alert == 'NONE' or not trader.attacker:exists() or
	(alert == 'SHIP_NEARBY' and ship:DistanceTo(trader.attacker) > 100) then
		trader['attacker'] = nil
		if trader.status == 'fleeing' then
			-- had not reached starport yet
			trader['status'] = 'inbound'
		elseif trader.status == 'cowering' then
			-- already reached starport and docked
			trader['status'] = 'docked'
			if trader.delay > Game.time then
				--[[ not ready to undock, so schedule it
				there is a slight chance that the status was changed while
				onShipDocked was in progress so fire a bit later ]]
				Timer:CallAt(trader.delay + 120, function () doUndock(ship) end)
			else
				-- ready to undock
				doUndock(ship)
			end
		end
	end
end
Event.Register("onShipAlertChanged", onShipAlertChanged)

local onShipHit = function (ship, attacker)
	if attacker == nil then return end-- XX

	-- XXX this whole thing might be better if based on amount of damage sustained
	if trade_ships[ship] == nil then return end
	local trader = trade_ships[ship]

	trader['chance'] = trader.chance or 0
	trader['chance'] = trader.chance + 0.1

	-- don't spam actions
	if trader.last_flee and Game.time - trader.last_flee < Engine.rand:Integer(5, 7) then return end

	-- if outbound jump now
	if trader.status == 'outbound' then
		if getSystemAndJump(ship) == 'OK' then
			return
		end
	end

	trader['status'] = 'fleeing'
	trader['attacker'] = attacker

	-- update last_flee
	trader['last_flee'] = Game.time

	-- if distance to starport is far attempt to hyperspace
	if trader.no_jump ~= true then
		if #starports == 0 then
			trader['no_jump'] = true -- it already tried in onEnterSystem
		elseif trader.starport and Engine.rand:Number(1) < trader.chance then
			local distance = ship:DistanceTo(trader.starport)
			if distance > 149598000 * (2 - trader.chance) then -- 149,598,000km = 1AU
				if getSystemAndJump(ship) then
					return
				else
					trader['no_jump'] = true
					trader['chance'] = trader.chance + 0.3
				end
			end
		end
	end

	-- maybe jettison a bit of cargo
	if Engine.rand:Number(1) < trader.chance then
		local cargo_type = nil
		local max_cap = ShipDef[ship.shipId].capacity
		for k, v in pairs(trader.cargo) do
			if v > 1 and Engine.rand:Number(1) < v / max_cap then
				cargo_type = k
				break
			end
		end
		if cargo_type and ship:Jettison(cargo_type) then
			trader.cargo[cargo_type] = trader.cargo[cargo_type] - 1
			Comms.ImportantMessage(attacker.label..', take this and leave us be, you filthy pirate!', ship.label)
			trader['chance'] = trader.chance - 0.1
		end
	end
end
Event.Register("onShipHit", onShipHit)

local onShipCollided = function (ship, other)
	if trade_ships[ship] == nil then return end
	if other:isa('CargoBody') then return end

	if other:isa('Ship') and other:IsPlayer() then
		onShipHit(ship, other)
		return
	end

	-- try to get away from body, onAICompleted will take over if we succeed
	ship:AIFlyTo(other)
end
Event.Register("onShipCollided", onShipCollided)

local onShipDestroyed = function (ship, attacker)
	if attacker == nil then return end-- XX
	if trade_ships[ship] ~= nil then
		local trader = trade_ships[ship]

		print(ship.label..' destroyed by '..attacker.label..', status:'..trader.status..' ship:'..trader.ship_name..', starport:'..(trader.starport and trader.starport.label or 'N/A'))
		trade_ships[ship] = nil

		if not attacker:isa("Ship") then
			spawnReplacement()
		end
		-- XXX consider spawning some CargoBodies if killed by a ship
	else
		for t_ship, trader in pairs(trade_ships) do
			if t_ship ~= 'interval' and trader.attacker and trader.attacker == ship then
				trader['attacker'] = nil
				if trader.status == 'fleeing' then
					-- had not reached starport yet
					trader['status'] = 'inbound'
				elseif trader.status == 'cowering' then
					-- already reached starport and docked
					trader['status'] = 'docked'

					if trader.delay > Game.time then
						--[[ not ready to undock, so schedule it
						there is a slight chance that the status was changed while
						onShipDocked was in progress so fire a bit later ]]
						Timer:CallAt(trader.delay + 120, function () doUndock(t_ship) end)
					else
						-- ready to undock
						doUndock(t_ship)
					end
				end
				return
			end
		end
	end
end
Event.Register("onShipDestroyed", onShipDestroyed)

local onGameStart = function ()
	-- create tables for data on the current system
	from_paths, starports, imports, exports = {}, {}, {}, {}

	system_updated = true

	if trade_ships == nil then
		-- create table to hold ships, keyed by ship object
		trade_ships = {}
		spawnInitialShips(true)
	else
		-- trade_ships was loaded by unserialize
		-- rebuild starports, imports and exports tables
		starports = Space.GetBodies(function (body) return body.superType == 'STARPORT' end)
		vacuum_starports = Space.GetBodies(function (body)
			return body.superType == 'STARPORT' and (body.type == 'STARPORT_ORBITAL' or (not body.path:GetSystemBody().parent.hasAtmosphere))
		end)
		if #starports == 0 then
			-- there are no starports so don't bother looking for goods
			return
		else
			for key,equip in pairs(e.cargo) do
				local v = Game.system:GetCommodityBasePriceAlterations(equip)
				if key ~= 'rubbish' and key ~= 'radioactives' and Game.system:IsCommodityLegal(equip) then
					if v > 2 then
						table.insert(imports, equip)
					elseif v < -2 then
						table.insert(exports, equip)
					end
				end
			end
		end

		-- rebuild nearby system paths for hyperspace spawns to come from
		local from_systems, dist = {}, 10
		while #from_systems < 10 do
			from_systems = Game.system:GetNearbySystems(dist)
			dist = dist + 5
		end
		from_paths = {}
		for _, system in ipairs(from_systems) do
			table.insert(from_paths, system.path)
		end

		-- check if any trade ships were waiting on a timer
		for ship, trader in pairs(trade_ships) do
			if ship ~= 'interval' and trader.delay and trader.delay > Game.time then
				if trader.status == 'docked' then
					Timer:CallAt(trader.delay, function () doUndock(ship) end)
				elseif trader.status == 'orbit' then
					Timer:CallAt(trader.delay, function ()
						if ship:exists() and ship.flightState ~= 'HYPERSPACE' then
							trader['starport'] = getNearestStarport(ship)
							ship:AIDockWith(trader.starport)
							trader['status'] = 'inbound'
							trader['delay'] = nil
						end
					end)
				end
			end
		end
	end
end
Event.Register("onGameStart", onGameStart)

local onGameEnd = function ()
	-- drop the references for our data so Lua can free them
	-- and so we can start fresh if the player starts another game
	trade_ships, system_updated, from_paths, starports, vacuum_starports, imports, exports = nil, nil, nil, nil, nil, nil, nil
end
Event.Register("onGameEnd", onGameEnd)

local serialize = function ()
	-- all we need to save is trade_ships, the rest can be rebuilt on load

	-- The serializer will crash if we try to serialize dead objects (issue #3123)
	-- also, trade_ships may be nil, because it is cleared in 'onGameEnd', and this may
	-- happen before the autosave module creates its '_exit' save
	if trade_ships ~= nil then
		local count = 0
		for k,v in pairs(trade_ships) do
			if type(k) == 'userdata' and not k:exists() then
				count = count + 1
				-- according to the Lua manual, removing items during iteration with pairs() or next() is ok
				trade_ships[k] = nil
			end
		end
		print('TradeShips: Removed ' .. count .. ' ships before serialization')
	end
	return trade_ships
end

local unserialize = function (data)
	trade_ships = data
end

Serializer:Register("TradeShips", serialize, unserialize)
