local addFuel = function (ship)
	local drive = ship:GetEquip('ENGINE', 0)

	-- a drive must be installed
	assert(drive ~= 'NONE', trade_ships[ship.label]['ship_name']..' '..ship.label..' has no drive!')

	-- the last character of the fitted drive is the class
	-- the fuel needed for max range is the square of the drive class
	local count = tonumber(string.sub(drive, -1)) ^ 2

	-- account for fuel it already has
	count = count - ship:GetEquipCount('CARGO', 'HYDROGEN')

	local added = ship:AddEquip('HYDROGEN', count)

	return added
end

local addShipEquip = function (ship)
	local trader = trade_ships[ship.label]
	local ship_type = ShipType.GetShipType(trader.ship_name)

	-- add standard equipment
	ship:AddEquip(ship_type.defaultHyperdrive)
	ship:AddEquip('ATMOSPHERIC_SHIELDING')
	ship:AddEquip('SCANNER')
	ship:AddEquip('AUTOPILOT')

	-- add defensive equipment based on lawlessness, luck and size
	local lawlessness = Game.system.lawlessness
	local size_factor = ship:GetEquipFree('CARGO') ^ 2 / 2000000

	if ship:GetEquipCount('CARGO', 'SHIELD_GENERATOR') == 0 and
	Engine.rand:Number(1) - 0.1 < lawlessness then
		local num = math.floor(math.sqrt(ship:GetEquipFree('CARGO') / 50))
		ship:AddEquip('SHIELD_GENERATOR', num)
		if ship_type:GetEquipSlotCapacity('ENERGYBOOSTER') > 0 and
		Engine.rand:Number(1) + 0.5 - size_factor < lawlessness then
			ship:AddEquip('SHIELD_ENERGY_BOOSTER')
		end
	end

	-- we can't use these yet
	if Engine.rand:Number(1) + 0.2 < lawlessness then
		ship:AddEquip('ECM_ADVANCED')
	elseif Engine.rand:Number(1) < lawlessness then
		ship:AddEquip('ECM_BASIC')
	end

	-- this should be rare
	if ship_type:GetEquipSlotCapacity('HULLAUTOREPAIR') > 0 and
	Engine.rand:Number(1) + 0.75 - size_factor < lawlessness then
		ship:AddEquip('HULL_AUTOREPAIR')
	end
end

local addShipCargo = function (ship, direction)
	local prices = Game.system:GetCommodityBasePriceAlterations()
	local added = 0
	local size_factor = ship:GetEquipFree('CARGO') / 50

	while ship:GetEquipFree('CARGO') > 0 do
		local cargo

		-- get random for direction
		if direction == 'import' then
			cargo = imports[Engine.rand:Integer(1, #imports)]
		else
			cargo = exports[Engine.rand:Integer(1, #exports)]
		end

		-- check if requires life support
		if cargo == 'LIVE_ANIMALS' or cargo == 'SLAVES' then
			ship:AddEquip('CARGO_LIFE_SUPPORT')
		end

		-- amount based on price and size of ship
		local num = math.abs(prices[cargo])
		num = Engine.rand:Integer(num * size_factor + 10, num * size_factor ^ 1.5 + 20)

		added = added + ship:AddEquip(cargo, num)
	end

	return added
end

local doUndock = function (ship)
	-- the player may have left the system or the ship may have already undocked
	if ship:exists() and ship:GetDockedWith() then
		if not ship:Undock() then
			-- unable to undock, try again in ten minutes
			Timer:CallAt(600, function ()
				doUndock(ship)
			end)
		end
	end
end

local getNearestStarport = function (ship)
	if #starports == 0 then return nil end
	if #starports == 1 then return starports[1] end

	local starport = starports[1]
	local distance = ship:DistanceTo(starport)
	for _, next_starport in ipairs(starports) do
		local next_distance = ship:DistanceTo(next_starport)
		if next_distance < distance then
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
	local cargo_list = ship:GetEquip('CARGO')
	local best_prices = 0

	-- find best system for cargo
	for _, next_system in ipairs(systems_in_range) do
		local prices = next_system:GetCommodityBasePriceAlterations()
		local next_prices = 0
		for _, cargo in ipairs(cargo_list) do
			if cargo ~= 'HYDROGEN' and cargo ~= 'SHIELD_GENERATOR' then
				next_prices = next_prices + prices[cargo]
			end
		end
		if next_prices >= best_prices then
			target_system, best_prices = next_system, next_prices
		end
	end

	if target_system == nil then
		-- find the closest system
		local distance = 1000000
		for _, next_system in ipairs(systems_in_range) do
			if Game.system:DistanceTo(next_system) < distance then
				target_system = next_system
			end
		end
	end

	-- XXX maybe pick a random starport, if there are any, and return path to it
	return target_system.path
end

local jumpToSystem = function (ship, target_path)
	if target_path == nil then return nil end

	local status, fuel, duration = ship:CanHyperspaceTo(target_path)
	-- make it so
	status = ship:HyperspaceTo(target_path)

	assert(status == 'OK', 'jumpToSystem:status is not OK')

	-- update table for ship
	trade_ships[ship.label]['status'] = 'hyperspace'
	trade_ships[ship.label]['starport'] = nil
	trade_ships[ship.label]['arrival'] = Game.time + duration
	trade_ships[ship.label]['arrival_system'] = target_path
	trade_ships[ship.label]['from_system'] = Game.system.path
	return status
end

local getSystemAndJump = function (ship)
	return jumpToSystem(ship, getSystem(ship))
end

local spawnInitialShips = function ()
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
	-- start with one ship per billion population
	local num_trade_ships = population
	-- add the lesser of import_score and export_score
	if import_score < export_score then
		num_trade_ships = num_trade_ships + import_score
	else
		num_trade_ships = num_trade_ships + export_score
	end
	-- vary by up to 50% more or fewer
	num_trade_ships = num_trade_ships * Engine.rand:Number(0.5, 1.5)
	-- reduce based on lawlessness
	num_trade_ships = num_trade_ships * (1 - lawlessness)
	-- compute distance and interval between ships
	local range = (9 / (num_trade_ships / 2))
	trade_ships['interval'] = (864000 / (num_trade_ships / 4))

	-- spawn the initial trade ships
	for i = 0, num_trade_ships do
		-- get the name of a ship, for example 'Imperial Courier'
		local ship_name = ship_names[Engine.rand:Integer(1, #ship_names)]
		local ship = nil

		if i < num_trade_ships / 4 then
			-- spawn the first quarter in port
			local starport = starports[Engine.rand:Integer(1, #starports)]

			ship = Space.SpawnShipDocked(ship_name, starport)
			if ship ~= nil then
				trade_ships[ship.label] = {
					status		= 'docked',
					starport	= starport,
					ship_name	= ship_name,
				}
			else
				-- the starport must have been full
				ship = Space.SpawnShipNear(ship_name, starport, 10000000, 149598000) -- 10mkm - 1AU
				trade_ships[ship.label] = {
					status		= 'inbound',
					starport	= starport,
					ship_name	= ship_name,
				}
			end
		elseif i < num_trade_ships / 4 * 3 then
			-- spawn the middle half in space
			local max_distance = range * (i - num_trade_ships / 4) + 2
			local min_distance = max_distance - range

			ship = Space.SpawnShip(ship_name, min_distance, max_distance)
			trade_ships[ship.label] = {
				status		= 'inbound',
				starport	= getNearestStarport(ship),
				ship_name	= ship_name,
			}
		else
			-- spawn the last quarter in hyperspace
			local min_time = trade_ships.interval * (i - num_trade_ships / 4 * 3)
			local max_time = min_time + trade_ships.interval
			local arrival = Game.time + Engine.rand:Integer(min_time, max_time)
			local local_systems = Game.system:GetNearbySystems(20)
			local from_system = local_systems[Engine.rand:Integer(1, #local_systems)]

			ship = Space.SpawnShip(ship_name, 9, 11, {from_system.path, arrival})
			trade_ships[ship.label] = {
				status			= 'hyperspace',
				arrival			= arrival,
				arrival_system	= Game.system.path,
				from_system		= from_system.path,
				ship_name		= ship_name,
			}
		end
		local trader = trade_ships[ship.label]

		-- add equipment and cargo
		addShipEquip(ship)
		local fuel_added = addFuel(ship)
		local direction = 'export'
		if trader.status ~= 'docked' then
			direction = 'import'
			-- remove fuel used to get here
			ship:RemoveEquip('HYDROGEN', Engine.rand:Integer(1, fuel_added))
		end
		local cargo_count = addShipCargo(ship, direction)

		-- give orders
		if trader.status == 'docked' then
			-- have ship wait 30-45 seconds per unit of cargo
			trader['delay'] = Game.time + (cargo_count * 30 * Engine.rand:Number(1, 1.5))
			Timer:CallAt(trader.delay, function ()
				doUndock(ship)
			end)
		elseif trader.status == 'inbound' then
			ship:AIDockWith(trader.starport)
		end
	end

	return num_trade_ships
end

local spawnReplacement = function ()
	-- spawn new ship in hyperspace
	if #starports > 0 and Game.system.population > 0 and #imports > 0 and #exports > 0 then
		local ship_names = ShipType.GetShipTypes('SHIP', function (t) return t.hullMass >= 100 end)
		local ship_name = ship_names[Engine.rand:Integer(1, #ship_names)]

		local arrival = Game.time + Engine.rand:Number(trade_ships.interval, trade_ships.interval * 2)
		local local_systems = Game.system:GetNearbySystems(20)
		local from_system = local_systems[Engine.rand:Integer(1, #local_systems)]

		local ship = Space.SpawnShip(ship_name, 9, 11, {from_system.path, arrival})
		trade_ships[ship.label] = {
			status			= 'hyperspace',
			arrival			= arrival,
			arrival_system	= Game.system.path,
			from_system		= from_system.path,
			ship_name		= ship_name,
		}

		addShipEquip(ship)
		local fuel_added = addFuel(ship)
		ship:RemoveEquip('HYDROGEN', Engine.rand:Integer(1, fuel_added))
		addShipCargo(ship, 'import')
	end
end

local updateTradeShipsTable = function ()
	local total, removed = 0, 0
	for label, trader in pairs(trade_ships) do
		total = total + 1
		if trader.status == 'hyperspace' then
			-- remove ships not coming here
			if trader.arrival_system ~= Game.system.path then
				trade_ships[label] = nil
				removed = removed + 1
			end
		else
			-- remove ships that are not in hyperspace
			trade_ships[label] = nil
			removed = removed + 1
		end
	end
	print('updateTSTable:total:'..total..',removed:'..removed)
end

local cleanTradeShipsTable = function ()
	local total, removed = 0, 0
	for label, trader in pairs(trade_ships) do
		if label ~= 'interval' then
			total = total + 1
			if trader.status == 'hyperspace' then
				-- remove well past due ships as the player can not catch them
				if trader.arrival + 86400 < Game.time then
					trade_ships[label] = nil
					removed = removed + 1
				end
			end
		end
	end
	print('cleanTSTable:total:'..total..',removed:'..removed)
end

local onEnterSystem = function (ship)
	if not ship:IsPlayer() then
		if trade_ships[ship.label] ~= nil then
			print(ship.label..' entered '..Game.system.name)
			if #starports == 0 then
				-- this only happens if player has followed ship to empty system

				getSystemAndJump(ship)
				-- if we couldn't reach any systems wait for player to attack
			else
				local starport = getNearestStarport(ship)
				ship:AIDockWith(starport)
				trade_ships[ship.label]['status'] = 'inbound'
				trade_ships[ship.label]['starport'] = starport
			end
		end
		return
	end

	updateTradeShipsTable()
	spawnInitialShips()
end
EventQueue.onEnterSystem:Connect(onEnterSystem)

local onLeaveSystem = function (ship)
	if ship:IsPlayer() then
		trade_ships['interval'] = nil
		local total, removed = 0, 0
		for label, trader in pairs(trade_ships) do
			total = total + 1
			if trader.status == 'hyperspace' then
				if trader.arrival_system == Game.system.path then
					-- remove ships that are in hyperspace to here
					trade_ships[label] = nil
					removed = removed + 1
				end
			else
				-- remove all ships that are not in hyperspace
				trade_ships[label] = nil
				removed = removed + 1
			end
		end
		print('onLeaveSystem:total:'..total..',removed:'..removed)
	elseif trade_ships[ship.label] ~= nil then
		print(ship.label..' left '..Game.system.name..' for '..trade_ships[ship.label]['arrival_system'])
		cleanTradeShipsTable()
		spawnReplacement()
	end
end
EventQueue.onLeaveSystem:Connect(onLeaveSystem)

local onFrameChanged = function (ship)
	if not ship:isa("Ship") or trade_ships[ship.label] == nil then return end
	local trader = trade_ships[ship.label]

	if trader.status == 'outbound' then
		-- the cloud inherits the ship velocity and vector
		ship:CancelAI()
		getSystemAndJump(ship)
	end
end
EventQueue.onFrameChanged:Connect(onFrameChanged)

local onShipDocked = function (ship)
	if trade_ships[ship.label] == nil then return end
	local trader = trade_ships[ship.label]

	if trader.status == 'fleeing' then
		trader['status'] = 'cowering'
	else
		trader['status'] = 'docked'
	end
	if trader.chance then
		trader['chance'] = trader.chance / 2
		trader['last_flee'], trader['no_jump'], trader['answered'] = nil, nil, nil
	end

	-- 'sell' trade cargo
	local cargo_count = 0
	local cargo_list = ship:GetEquip('CARGO')
	for i = 1, #cargo_list do
		local cargo = cargo_list[i]
		if cargo ~= 'HYDROGEN' and cargo ~= 'SHIELD_GENERATOR' then
			cargo_count = cargo_count + ship:RemoveEquip(cargo, 1000000)
		end
	end

	addFuel(ship)
	cargo_count = cargo_count + addShipCargo(ship, 'export')

	-- delay undocking by 30-45 seconds for every unit of cargo transfered
	trader['delay'] = Game.time + (cargo_count * 30 * Engine.rand:Number(1, 1.5))
	if trader.status == 'docked' then
		print(ship.label..' will undock at '..Format.Date(trader.delay))
		Timer:CallAt(trader.delay, function ()
			doUndock(ship)
		end)
	end
end
EventQueue.onShipDocked:Connect(onShipDocked)

local onShipUndocked = function (ship, starport)
	if trade_ships[ship.label] == nil then return end

	ship:AIFlyTo(starport)

	trade_ships[ship.label]['status'] = 'outbound'
end
EventQueue.onShipUndocked:Connect(onShipUndocked)

local onAICompleted = function (ship)
	if trade_ships[ship.label] == nil then return end
	print(ship.label..' AICompleted')
	-- XXX if police that we spawned then attack ship that attacked trader

	local trader = trade_ships[ship.label]
	if trader.status == 'outbound' then
		getSystemAndJump(ship)
	elseif trader.status == 'orbit' then
		trader['starport'] = getNearestStarport(ship)
		ship:AIDockWith(trader.starport)
		trader['status'] = 'inbound'
	elseif trader.status == 'inbound' then
		-- assuming here that the starport is full
		-- get parent body of starport and orbit
		local sbody = trader.starport.path:GetSystemBody()
		ship:AIEnterHighOrbit(sbody.parent)
		trader['status'] = 'orbit'
	end
end
EventQueue.onAICompleted:Connect(onAICompleted)

local onShipAlertChanged = function (ship, alert)
	if trade_ships[ship.label] == nil then return end
	print(ship.label..' alert changed to '..alert)
	local trader = trade_ships[ship.label]
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
				Timer:CallAt(trader.delay + 120, function ()
					doUndock(ship)
				end)
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
	if trade_ships[ship.label] == nil then return end
	local trader = trade_ships[ship.label]

	trader['chance'] = trader.chance or 0
	trader['chance'] = trader.chance + 0.1

	-- don't spam actions
	if trader.last_flee and Game.time - trader.last_flee < 60 then return end

	-- if outbound jump now
	if trader.status == 'outbound' then getSystemAndJump(ship) end

	trader['status'] = 'fleeing'
	trader['attacker'] = attacker

	-- if distance to starport is far attempt to hyperspace
	if trader.no_jump ~= true then
		if #starports == 0 then
			trader['no_jump'] = true -- it already tried in onEnterSystem
		elseif Engine.rand:Number(1) < trader.chance then
			local distance = ship:DistanceTo(trader.starport)
			if distance > 149598000 * (2 - trader.chance) then -- 149,598,000km = 1AU
				local target_system = getSystem(ship, 'fleeing')
				if target_system ~= nil then
					jumpToSystem(ship, target_system)
					return
				else
					trader['no_jump'] = true
					trader['chance'] = trader.chance + 0.3
				end
			end
		end
	end

	-- update last_flee
	trader['last_flee'] = Game.time
	-- close or jump failed, call for help until answered
	if #starports > 0 and trader.answered ~= true then
		UI.ImportantMessage('MAYDAY! MAYDAY! MAYDAY! We are under attack and require assistance!', ship.label)
		-- the closer to the starport the better the chance of being answered
		if Engine.rand:Number(1) > trader.starport / 1196784000 * trader.chance then -- 8AU
			trader['answered'] = true
			UI.ImportantMessage('Roger '..ship.label..', assistance is on the way!', trader.starport.label)
			trader['chance'] = 0
			-- XXX spawn some sort of police and fly to trader
			return
		end
	end

	-- maybe jettison a bit of cargo
	if Engine.rand:Number(1) < trader.chance then
		local cargo_list = ship:GetEquip('CARGO')
		if #cargo_list == 0 then return end

		local cargo = cargo_list[Engine.rand:Integer(1, #cargo_list)]
		if cargo ~= 'HYDROGEN' and cargo ~= 'SHIELD_GENERATOR' and ship:Jettison(cargo) then
			UI.ImportantMessage(attacker.label..', take this and leave us be, you filthy pirate!', ship.label)
			trader['chance'] = trader.chance - 0.1
		end
	end
end
EventQueue.onShipHit:Connect(onShipHit)

local onShipCollided = function (ship, other)
	if trade_ships[ship.label] == nil then return end
	if other:isa('CargoBody') then return end
	print(ship.label..' collided with '..other.label)
	
	if other:isa('Ship') and other:IsPlayer() then
		onShipHit(ship, other)
		return
	end

	-- try to get away from body, onAICompleted will take over if we succeed
	ship:AIFlyTo(other)
end
EventQueue.onShipCollided:Connect(onShipCollided)

local onShipDestroyed = function (ship, attacker)
	if trade_ships[ship.label] ~= nil then
		-- XXX consider spawning some CargoBodies

		trade_ships[ship.label] = nil
		print(ship.label..' destroyed by '..attacker.label)

		if not attacker:isa("Ship") then
			spawnReplacement()
		end
	end
end
EventQueue.onShipDestroyed:Connect(onShipDestroyed)

local onGameStart = function ()
	-- create tables for data on the current system
	starports, imports, exports = {}, {}, {}

	if trade_ships == nil then
		-- create table to hold ships, keyed by label (ex. OD-7764)
		trade_ships = {}
		spawnInitialShips()
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
	end
end
EventQueue.onGameStart:Connect(onGameStart)

local onGameEnd = function ()
	-- drop the references for our data so Lua can free them
	-- and so we can start fresh if the player starts another game
	trade_ships, starports, imports, exports = nil, nil, nil, nil
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
