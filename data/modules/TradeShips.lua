local addFuel = function (ship)
	local drive = ship:GetEquip('ENGINE', 0)

	-- a drive must be installed
	assert(drive ~= 'NONE', trade_ships[ship.label]['ship_name']..' '..ship.label..' has no drive!')

	-- the last character of the fitted drive is the class
	-- the fuel needed for max range is the square of the drive class
	local count = tonumber(string.sub(drive, -1)) ^ 2
	local added = ship:AddEquip('HYDROGEN', count)

	print(ship.label..' added '..added..' of '..count..' fuel')

	return added
end

local addShipContents = function (ship)
	local trader = trade_ships[ship.label]
	-- add equipment
	local ship_type = ShipType.GetShipType(trader.ship_name)
	ship:AddEquip(ship_type.defaultHyperdrive)
	ship:AddEquip('ATMOSPHERIC_SHIELDING')
	-- XXX could add more, like defenses, based on current or arg system
	-- add cargo
	if trader.cargo == 'LIVE_ANIMALS' or trader.cargo == 'SLAVES' then
		ship:AddEquip('CARGO_LIFE_SUPPORT')
	end
	-- let AddEquip add as many as it can
	local added = ship:AddEquip(trader.cargo, 1000000)

	print(ship.label..' added '..added..' cargo of '..trader.cargo)

	return added
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

local getSystem = function (ship, cargo)
	-- cargo is optional and will use cargo value in trade_ships table if not given
	local stats = ship:GetStats()
	local systems_in_range = Game.system:GetNearbySystems(stats.hyperspaceRange)
	if #systems_in_range == 0 then return nil end
	if #systems_in_range == 1 then
		return systems_in_range[1].path
	end

	cargo =	cargo or trade_ships[ship.label]['cargo']

	assert(cargo ~= nil, 'getSystem:cargo is nil')
	assert(cargo ~= '', 'getSystem:cargo is null')

	local target_system = nil
	if cargo == 'emergency' then
		-- find the closest system
		local distance = 1000000
		for _, next_system in ipairs(systems_in_range) do
			if Game.system:DistanceTo(next_system) < distance then
				target_system = next_system
			end
		end
		-- XXX should only use this if can get back out of target system
		-- which means checking for fuel or starports to buy it from
	else
		-- find best system for cargo
		local best_price = 0
		for _, next_system in ipairs(systems_in_range) do
			local prices = next_system:GetCommodityBasePriceAlterations()
			if prices[cargo] >= best_price then
				best_price = prices[cargo]
				target_system = next_system
			end
		end
	end

	assert(target_system.path ~= nil, 'getSystem:target_system.path is nil,system:'..target_system.name)

	-- XXX maybe pick a random starport, if there are any, and return path to it
	return target_system.path
end

local jumpToSystem = function (ship, target_path)
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
	-- must only be used if can't fail
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
		if Game.system:IsCommodityLegal(k) then
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
					cargo 		= exports[Engine.rand:Integer(1, #exports)],
				}
				local ship_type = ShipType.GetShipType(ship_name)
				ship:AddEquip(ship_type.defaultHyperdrive)
				addFuel(ship)
			else
				-- the starport must have been full
				ship = Space.SpawnShipNear(ship_name, starport, 10000000, 149598000) -- 10mkm - 1AU
				trade_ships[ship.label] = {
					status		= 'inbound',
					starport	= starport,
					ship_name	= ship_name,
					cargo 		= imports[Engine.rand:Integer(1, #imports)],
				}
			end
		elseif i < num_trade_ships / 4 * 3 then
			-- spawn the middle half in space
			local max_distance = range * (i - num_trade_ships / 4) + 2
			local min_distance = max_distance - range

			print('distance min:'..min_distance..' max:'..max_distance)

			ship = Space.SpawnShip(ship_name, min_distance, max_distance)
			trade_ships[ship.label] = {
				status		= 'inbound',
				starport	= getNearestStarport(ship),
				ship_name	= ship_name,
				cargo 		= imports[Engine.rand:Integer(1, #imports)],
			}
		else
			-- spawn the last quarter in hyperspace
			local max_time = trade_ships.interval * (i - num_trade_ships / 4)
			local min_time = max_time - trade_ships.interval

			print('time min:'..min_time..' max:'..max_time)

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
				cargo 			= imports[Engine.rand:Integer(1, #imports)],
			}
		end
		local trader = trade_ships[ship.label]

		-- add equipment and cargo
		addShipContents(ship)
		if trader.status ~= 'docked' then
			-- make space for fuel used to get here
			ship:RemoveEquip(trader.cargo, 8)
		end

		-- give orders
		if trader.status == 'docked' then
			-- have ship wait 30-45 seconds per unit of cargo
			local cargo_count = ship:GetEquipCount('CARGO', trader.cargo)
			Timer:CallAt(Game.time + (cargo_count * 30 * Engine.rand:Number(1, 1.5)), function ()
				if ship:exists() then -- player may have left system in meantime
					ship:Undock()
				end
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
			cargo 			= imports[Engine.rand:Integer(1, #imports)],
		}
		addShipContents(ship)
		ship:RemoveEquip(trade_ships[ship.label]['cargo'], 8)
	end
end

local updateTradeShipsTable = function ()
	local total, removed = 0, 0
	for label, trader in pairs(trade_ships) do
		if label ~= 'attacker' and label ~= 'interval' then
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
	end
	print('updateTSTable:total:'..total..',removed:'..removed)
end

local cleanTradeShipsTable = function ()
	if stop_clean == true then return true end

	local total, removed = 0, 0
	for label, trader in pairs(trade_ships) do
		if label ~= 'attacker' and label ~= 'interval' then
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
	return stop_clean
end

local onEnterSystem = function (ship)
	if not ship:IsPlayer() then
		if trade_ships[ship.label] ~= nil then
			print(ship.label..' entered '..Game.system.name)
			if #starports == 0 then
				-- this only happens if player has followed ship to empty system
				local target_system = getSystem(ship)
				if target_system ~= nil then
					jumpToSystem(ship, target_system)
				end
				-- if we couldn't reach any systems we're fucked
				-- wait for player to attack and let onShipHit take over
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
	if spawnInitialShips() then
		-- the system was fit for trading, schedule housekeeping
		stop_clean = false
		Timer:CallEvery(86400, cleanTradeShipsTable) -- 24 hours
	end
end
EventQueue.onEnterSystem:Connect(onEnterSystem)

local onLeaveSystem = function (ship)
	if ship:IsPlayer() then
		stop_clean = true
		trade_ships['interval'] = nil
		trade_ships['attacker'] = nil
		local total, removed = 0, 0
		for label, trader in pairs(trade_ships) do
			if label ~= 'attacker' and label ~= 'interval' then
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
		end
		print('onLeaveSystem:total:'..total..',removed:'..removed)
	elseif trade_ships[ship.label] ~= nil then
		print(ship.label..' left '..Game.system.name)
		spawnReplacement()
	end
end
EventQueue.onLeaveSystem:Connect(onLeaveSystem)

local onFrameChanged = function (ship)
	if not ship:isa("Ship") or trade_ships[ship.label] == nil then return end
	local trader = trade_ships[ship.label]
	if trader.status == 'outbound' then
		-- a problem with this is that the cloud inherits the ship velocity and vector
		ship:CancelAI()
		getSystemAndJump(ship)
	elseif trader.status == 'inbound' then
		-- no need to cancel, issuing a new AI order does that 
		ship:AIDockWith(trader.starport)
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

	-- 'sell' trade cargo
	local cargo_count = ship:RemoveEquip(trader.cargo, 1000000)

	addFuel(ship)

	trader['cargo'] = exports[Engine.rand:Integer(1, #exports)]
	
	if trader.cargo == 'LIVE_ANIMALS' or trader.cargo == 'SLAVES' then
		ship:AddEquip('CARGO_LIFE_SUPPORT')
	end
	-- let AddEquip add as many as it can
	cargo_count = cargo_count + ship:AddEquip(trader.cargo, 1000000)

	-- delay undocking by 30-45 seconds for every unit of cargo transfered
	trader['delay'] = Game.time + (cargo_count * 30 * Engine.rand:Number(1, 1.5))
	if trader.status == 'docked' then
		print(ship.label..' will undock at '..Format.Date(trader.delay))
		Timer:CallAt(trader.delay, function ()
			if ship:exists() and ship:GetDockedWith() ~= nil then -- player may have left system in meantime
				ship:Undock()
			end
		end)
	end
end
EventQueue.onShipDocked:Connect(onShipDocked)

local onShipUndocked = function (ship, starport)
	if trade_ships[ship.label] == nil then return end

	print(ship.label..' undocked from '..starport.label)

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
		-- wait 6 hours and try docking again
		trader['status'] = 'inbound'
		Timer:CallAt(Game.time + 21600, function ()
			if ship:exists() then -- player may have left system in meantime
				ship:AIDockWith(trader.starport)
			end
		end)
	elseif trader.status == 'inbound' then
		-- assuming here that the starport is full
		-- find nearby starports
		local nearby = {}
		for _, starport in ipairs(starports) do
			if trader.starport ~= starport then
				if trader.starport:DistanceTo(starport) < 1000000 then
					table.insert(nearby, starport)
				end
			end
		end
		-- pick one and try to dock with it
		if #nearby > 1 then
			trader['starport'] = nearby[Engine.rand:Integer(1, #nearby)]
			ship:AIDockWith(trader.starport)
		elseif #nearby == 1 then
			trader['starport'] = nearby[1]
			ship:AIDockWith(trader.starport)
		else
			-- there are none nearby
			-- get parent body of starport and orbit
			-- XXX this should use the new SystemBody.parent
			local bodies = Space.GetBodies(function (body) return body.isa == 'Planet' end)
			local closest, parent = 100000000
			for _, body in ipairs(bodies) do
				local current = trader.starport:DistanceTo(body)
				if current < closest then
					parent, closest = body, current
				end
			end
			if parent ~= nil then -- if it is nil let the poor bastard crash for now
				ship:AIEnterMediumOrbit(parent)
				trader['status'] = 'orbit'
			end
		end
	end
end
EventQueue.onAICompleted:Connect(onAICompleted)

local onShipAlertChanged = function (ship, alert)
	if trade_ships[ship.label] == nil then return end
	print(ship.label..' alert changed to '..alert)
	local trader = trade_ships[ship.label]

	if alert == 'NONE' then
		if trader.status == 'fleeing' then
			-- had not reached starport yet
			trader['status'] = 'inbound'
		elseif trader.status == 'cowering' then
			-- already reached starport and docked
			trader['status'] = 'docked'
			if trader.delay > Game.time then
				--[[ not ready to undock, so schedule it
				there is a slight chance that the status was changed while onShipDocked
				was in progress so fire a bit later and check if already undocked ]]
				Timer:CallAt(trader.delay + 120, function ()
					if ship:exists() and ship:GetDockedWith() ~= nil then
						ship:Undock()
					end
				end)
			else
				-- ready to undock
				if ship:GetDockedWith() ~= nil then
					ship:Undock()
				end
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
	trade_ships['attacker'] = attacker

	-- if distance to starport is far attempt to hyperspace
	if trader.no_jump ~= true then
		local attempt_jump = false
		if #starports == 0 then
			trader['no_jump'] = true -- it already tried in onEnterSystem
		elseif Engine.rand:Number(1) < trader.chance then
			local distance = ship:DistanceTo(trader.starport)
			if distance > 149598000 * (2 - trader.chance) then -- 149,598,000km = 1AU
				local target_system = getSystem(ship, 'emergency')
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
		if ship:Jettison(trader.cargo) then
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
		--onShipHit(ship, other)
		return
	end

	-- try to get away from body, onAICompleted will take over if we succeed
	ship:AIFlyTo(other)

	-- XXX could find closest non-starport body and orbit
end
EventQueue.onShipCollided:Connect(onShipCollided)

local onShipDestroyed = function (ship, attacker)
	if trade_ships[ship.label] ~= nil then
		-- XXX consider spawning some CargoBodies
		trade_ships[ship.label] = nil
		print(ship.label..' destroyed by '..attacker.label)
		spawnReplacement()
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
				if Game.system:IsCommodityLegal(k) then
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
