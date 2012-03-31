--[[
	trade_ships
		interval - is minimum amount of time between hyperspace arrivals,
			stored here as it needs to be saved; number as seconds, updated by
			spawnInitialShips
		ship - object returned from Space:SpawnShip*
			ship_name - of this ship type; string
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

	imports, exports - in the current system, indexed array of
		Constants.EquipType strings, updated by spawnInitialShips
--]]
local trade_ships, system_updated, from_paths, starports, imports, exports

local addFuel = function (ship)
	local drive = ship:GetEquip('ENGINE', 1)

	-- a drive must be installed
	if drive == 'NONE' then
		print(trade_ships[ship]['ship_name']..' has no drive!')
		return nil
	end

	-- the last character of the fitted drive is the class
	-- the fuel needed for max range is the square of the drive class
	local count = tonumber(string.sub(drive, -1)) ^ 2

	-- account for fuel it already has
	count = count - ship:GetEquipCount('CARGO', 'HYDROGEN')

	local added = ship:AddEquip('HYDROGEN', count)

	return added
end

local addShipEquip = function (ship)
	local trader = trade_ships[ship]
	local ship_type = ShipType.GetShipType(trader.ship_name)

	-- add standard equipment
	ship:AddEquip(ship_type.defaultHyperdrive)
	ship:AddEquip('ATMOSPHERIC_SHIELDING')
	ship:AddEquip('SCANNER')
	ship:AddEquip('AUTOPILOT')
	ship:AddEquip('CARGO_LIFE_SUPPORT')

	local stats = ship:GetStats()

	-- add defensive equipment based on lawlessness, luck and size
	local lawlessness = Game.system.lawlessness
	local size_factor = stats.freeCapacity ^ 2 / 2000000

	if Engine.rand:Number(1) - 0.1 < lawlessness then
		local num = math.floor(math.sqrt(stats.freeCapacity / 50)) -
					 ship:GetEquipCount('SHIELD', 'SHIELD_GENERATOR')
		if num > 0 then ship:AddEquip('SHIELD_GENERATOR', num) end
		if ship_type:GetEquipSlotCapacity('ENERGYBOOSTER') > 0 and
		Engine.rand:Number(1) + 0.5 - size_factor < lawlessness then
			ship:AddEquip('SHIELD_ENERGY_BOOSTER')
		end
	end

	-- we can't use these yet
	if ship_type:GetEquipSlotCapacity('ECM') > 0 then
		if Engine.rand:Number(1) + 0.2 < lawlessness then
			ship:AddEquip('ECM_ADVANCED')
		elseif Engine.rand:Number(1) < lawlessness then
			ship:AddEquip('ECM_BASIC')
		end
	end

	-- this should be rare
	if ship_type:GetEquipSlotCapacity('HULLAUTOREPAIR') > 0 and
	Engine.rand:Number(1) + 0.75 - size_factor < lawlessness then
		ship:AddEquip('HULL_AUTOREPAIR')
	end
end

local addShipCargo = function (ship, direction)
	local prices = Game.system:GetCommodityBasePriceAlterations()
	local total = 0
	local empty_space = ship:GetStats().freeCapacity
	local size_factor = empty_space / 20
	local cargo = {}

	if direction == 'import' and #imports == 1 then
		total = ship:AddEquip(imports[1], empty_space)
		cargo[imports[1]] = total
	elseif direction == 'export' and #exports == 1 then
		total = ship:AddEquip(exports[1], empty_space)
		cargo[exports[1]] = total
	elseif (direction == 'import' and #imports > 1) or
			(direction == 'export' and #exports > 1) then
		while total < empty_space do
			local cargo_type

			-- get random for direction
			if direction == 'import' then
				cargo_type = imports[Engine.rand:Integer(1, #imports)]
			else
				cargo_type = exports[Engine.rand:Integer(1, #exports)]
			end

			-- amount based on price and size of ship
			local num = math.abs(prices[cargo_type]) * size_factor
			num = Engine.rand:Integer(num, num * 2)

			local added = ship:AddEquip(cargo_type, num)
			if cargo[cargo_type] == nil then
				cargo[cargo_type] = added
			else
				cargo[cargo_type] = cargo[cargo_type] + added
			end
			total = total + added
		end
	end
	-- if the table for direction was empty then cargo is empty and total is 0

	trade_ships[ship]['cargo'] = cargo
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
	if #starports == 1 then return starports[1] end

	local starport = starports[1]
	local distance = ship:DistanceTo(starport)
	for _, next_starport in ipairs(starports) do
		local next_distance = ship:DistanceTo(next_starport)
		if next_distance < distance and next_starport ~= current then
			starport, distance = next_starport, next_distance
		end
	end

	return starport
end

local getSystem = function (ship)
	local stats = ship:GetStats()
	local systems_in_range = Game.system:GetNearbySystems(stats.hyperspaceRange)
	if #systems_in_range == 0 then return nil end
	if #systems_in_range == 1 then
		return systems_in_range[1].path
	end

	local target_system = nil
	local best_prices = 0

	-- find best system for cargo
	for _, next_system in ipairs(systems_in_range) do
		if #next_system:GetStationPaths() > 0 then
			local prices = next_system:GetCommodityBasePriceAlterations()
			local next_prices = 0
			for cargo, count in pairs(trade_ships[ship]['cargo']) do
				next_prices = next_prices + (prices[cargo] * count)
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
		local systems_half_range = Game.system:GetNearbySystems(stats.hyperspaceRange / 2)

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

	local status, fuel, duration = ship:HyperspaceTo(target_path)

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

local spawnInitialShips = function (game_start)
	-- check if the current system can be traded in
	starports = Space.GetBodies(function (body) return body.superType == 'STARPORT' end)
	if #starports == 0 then return nil end
	local population = Game.system.population
	if population == 0 then return nil end
	local ship_names = ShipType.GetShipTypes('SHIP', function (t) return t.hullMass >= 100 end)
	if #ship_names == 0 then return nil end

	-- get a measure of the market size and build lists of imports and exports
	local prices = Game.system:GetCommodityBasePriceAlterations()
	local import_score, export_score = 0, 0
	imports, exports = {}, {}
	for k,v in pairs(prices) do
		if k ~= 'RUBBISH' and k ~= 'RADIOACTIVES' and Game.system:IsCommodityLegal(k) then
			-- values from SystemInfoView::UpdateEconomyTab
			if		v > 10	then
				import_score = import_score + 2
			elseif	v > 2	then
				import_score = import_score + 1
				table.insert(imports, k)
			elseif	v < -10	then
				export_score = export_score + 2
			elseif	v < -2	then
				export_score = export_score + 1
				table.insert(exports, k)
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
		dist = dist + 5
		from_systems = Game.system:GetNearbySystems(dist)
	end
	from_paths = {}
	for _, system in ipairs(from_systems) do
		table.insert(from_paths, system.path)
	end

	-- spawn the initial trade ships
	for i = 0, num_trade_ships do
		-- get the name of a ship, for example 'Imperial Courier'
		local ship_name = ship_names[Engine.rand:Integer(1, #ship_names)]
		local ship = nil

		if game_start and i < num_trade_ships / 4 then
			-- spawn the first quarter in port if at game start
			local starport = starports[Engine.rand:Integer(1, #starports)]

			ship = Space.SpawnShipDocked(ship_name, starport)
			if ship ~= nil then
				trade_ships[ship] = {
					status		= 'docked',
					starport	= starport,
					ship_name	= ship_name,
				}
			else
				-- the starport must have been full
				ship = Space.SpawnShipNear(ship_name, starport, 10000000, 149598000) -- 10mkm - 1AU
				trade_ships[ship] = {
					status		= 'inbound',
					starport	= starport,
					ship_name	= ship_name,
				}
			end
		elseif i < num_trade_ships * 0.75 then
			-- spawn the first three quarters in space, or middle half if game start
			local min_dist = range * i + 1
			if game_start then
				min_dist = min_dist - (range * (num_trade_ships / 4))
			end

			ship = Space.SpawnShip(ship_name, min_dist, min_dist + range)
			trade_ships[ship] = {
				status		= 'inbound',
				starport	= getNearestStarport(ship),
				ship_name	= ship_name,
			}
		else
			-- spawn the last quarter in hyperspace
			local min_time = trade_ships.interval * (i - num_trade_ships * 0.75)
			local max_time = min_time + trade_ships.interval
			local dest_time = Game.time + Engine.rand:Integer(min_time, max_time)
			local from = from_paths[Engine.rand:Integer(1, #from_paths)]

			ship = Space.SpawnShip(ship_name, 9, 11, {from, dest_time})
			trade_ships[ship] = {
				status		= 'hyperspace',
				dest_time	= dest_time,
				dest_path	= Game.system.path,
				from_path	= from,
				ship_name	= ship_name,
			}
		end
		local trader = trade_ships[ship]

		-- add equipment and cargo
		addShipEquip(ship)
		local fuel_added = addFuel(ship)
		if trader.status == 'docked' then
			local delay = fuel_added + addShipCargo(ship, 'export')
			-- have ship wait 30-45 seconds per unit of cargo
			trader['delay'] = Game.time + (delay * Engine.rand:Number(30, 45))
			Timer:CallAt(trader.delay, function () doUndock(ship) end)
		else
			addShipCargo(ship, 'import')
			-- remove fuel used to get here
			if fuel_added and fuel_added > 0 then
				ship:RemoveEquip('HYDROGEN', Engine.rand:Integer(1, fuel_added)) end
			if trader.status == 'inbound' then ship:AIDockWith(trader.starport) end
		end
	end

	return num_trade_ships
end

local spawnReplacement = function ()
	-- spawn new ship in hyperspace
	if #starports > 0 and Game.system.population > 0 and #imports > 0 and #exports > 0 then
		local ship_names = ShipType.GetShipTypes('SHIP', function (t) return t.hullMass >= 100 end)
		local ship_name = ship_names[Engine.rand:Integer(1, #ship_names)]

		local dest_time = Game.time + Engine.rand:Number(trade_ships.interval, trade_ships.interval * 2)
		local from = from_paths[Engine.rand:Integer(1, #from_paths)]

		local ship = Space.SpawnShip(ship_name, 9, 11, {from, dest_time})
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
			ship:RemoveEquip('HYDROGEN', Engine.rand:Integer(1, fuel_added))
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
		if #starports == 0 then
			-- this only happens if player has followed ship to empty system

			getSystemAndJump(ship)
			-- if we couldn't reach any systems wait for player to attack
		else
			local starport = getNearestStarport(ship)
			ship:AIDockWith(starport)
			trade_ships[ship]['starport'] = starport
			trade_ships[ship]['status'] = 'inbound'
		end
	end
end
EventQueue.onEnterSystem:Connect(onEnterSystem)

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
EventQueue.onLeaveSystem:Connect(onLeaveSystem)

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
EventQueue.onFrameChanged:Connect(onFrameChanged)

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

	local damage = ShipType.GetShipType(trader.ship_name).hullMass -
					ship:GetStats().hullMassLeft
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
EventQueue.onShipDocked:Connect(onShipDocked)

local onShipUndocked = function (ship, starport)
	if trade_ships[ship] == nil then return end

	-- fly to the limit of the starport frame
	ship:AIFlyTo(starport)

	trade_ships[ship]['status'] = 'outbound'
end
EventQueue.onShipUndocked:Connect(onShipUndocked)

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
				if ship:exists() then
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
EventQueue.onAICompleted:Connect(onAICompleted)

local onShipLanded = function (ship, body)
	if trade_ships[ship] == nil then return end
	print(ship.label..' Landed: '..trade_ships[ship].starport.label)

	doOrbit(ship)
end
EventQueue.onShipLanded:Connect(onShipLanded)

local onShipAlertChanged = function (ship, alert)
	if trade_ships[ship] == nil then return end
	if alert ~= 'NONE' then
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
EventQueue.onShipAlertChanged:Connect(onShipAlertChanged)

local onShipHit = function (ship, attacker)
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
		elseif Engine.rand:Number(1) < trader.chance then
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
		local max_cap = ship:GetStats().maxCapacity
		for k, v in pairs(trader.cargo) do
			if v > 1 and Engine.rand:Number(1) < v / max_cap then
				cargo_type = k
				break
			end
		end
		if cargo_type and ship:Jettison(cargo_type) then
			trader.cargo[cargo_type] = trader.cargo[cargo_type] - 1
			UI.ImportantMessage(attacker.label..', take this and leave us be, you filthy pirate!', ship.label)
			trader['chance'] = trader.chance - 0.1
		end
	end
end
EventQueue.onShipHit:Connect(onShipHit)

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
EventQueue.onShipCollided:Connect(onShipCollided)

local onShipDestroyed = function (ship, attacker)
	if trade_ships[ship] ~= nil then
		local trader = trade_ships[ship]

		print(ship.label..' destroyed by '..attacker.label..', status:'..trader.status..' ship:'..trader.ship_name..', starport:'..trader.starport.label)
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
EventQueue.onShipDestroyed:Connect(onShipDestroyed)

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
		if #starports == 0 then
			-- there are no starports so don't bother looking for goods
			return
		else
			local prices = Game.system:GetCommodityBasePriceAlterations()
			for k,v in pairs(prices) do
				if k ~= 'RUBBISH' and k ~= 'RADIOACTIVES' and Game.system:IsCommodityLegal(k) then
					if v > 2 then
						table.insert(imports, k)
					elseif v < -2 then
						table.insert(exports, k)
					end
				end
			end
		end

		-- rebuild nearby system paths for hyperspace spawns to come from
		local from_systems, dist = {}, 10
		while #from_systems < 10 do
			dist = dist + 5
			from_systems = Game.system:GetNearbySystems(dist)
		end
		from_paths = {}
		for _, system in ipairs(from_systems) do
			table.insert(from_paths, system.path)
		end

		-- check if any trade ships were waiting on a timer
		for ship, trader in ipairs(trade_ships) do
			if trader.delay > Game.time then
				if trader.status == 'docked' then
					Timer:CallAt(trader.delay, function () doUndock(ship) end)
				elseif trader.status == 'orbit' then
					Timer:CallAt(trader.delay, function ()
						if ship:exists() then
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
EventQueue.onGameStart:Connect(onGameStart)

local onGameEnd = function ()
	-- drop the references for our data so Lua can free them
	-- and so we can start fresh if the player starts another game
	trade_ships, system_updated, from_paths, starports, imports, exports = nil, nil, nil, nil, nil, nil
end
EventQueue.onGameEnd:Connect(onGameEnd)

local serialize = function ()
	-- all we need to save is trade_ships, the rest can be rebuilt on load
	return trade_ships
end

local unserialize = function (data)
	trade_ships = data
end

Serializer:Register("TradeShips", serialize, unserialize)
