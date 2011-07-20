-- TODO: Add event for clouds that are gone
--	 saving while hyperspacing is not going to work
--	 call for help when attacked

local max_trader_dist = 20 -- don't produce clouds for further than this many light years away
local max_trader_pick_dist = 12 -- max distance in light years that trader can pick
local min_trader_hull_mass = 55
local traders_per_population = 3 -- how many traders will be added per billion population

local min_wait = 2*60*60
local max_wait = 21*60*60

local disable_periodic = 0

local traders = {}

local traders_count = 0

-- poor trader never get any money
local _add_items = function (trader, factor)
	local capacity = trader.ship:GetEquipFree('CARGO')
	if capacity == 0 then print(trader.ship.label .. " CARGO full") return end
	local base_price_alterations = Game.system:GetCommodityBasePriceAlterations()

	for k,v in pairs(base_price_alterations) do
		if (v * factor) > 5 then
			if Game.system:IsCommodityLegal(k) then
				local chance = math.min(math.abs(v) * 2, 100)

				if Engine.rand:Integer(1, 100) <= chance then
					local count = trader.ship:AddEquip(k, Engine.rand:Integer(math.abs(v), v*v))

					capacity = capacity - count
					print(trader.ship.label .. " got " .. count .. "t of " .. k)
				end
			end
		end
		if capacity < 1 then
			print(trader.ship.label .. " CARGO full")
			return
		end
	end

end

local _hyperspace = function (trader)
	local ret = trader.ship:HyperspaceTo(trader.dest)

	if ret == 'OK' then
		print(trader.ship.label .. " hyperspace")
		trader.state = 'HYPERSPACE'
		return 0
	end
	return 1
end

local _remove_items = function (trader)
	local equiplist = trader.ship:GetEquip('CARGO')

	for i = 1,#equiplist do
		trader.ship:RemoveEquip(equiplist[i], 9999)
	end
end

local _random_station = function ()
	local stations = Space.GetBodies(function (body) return body:isa("SpaceStation") end)
	if #stations == 0 then return nil end

	return stations[Engine.rand:Integer(1,#stations)]
end

local _refuel = function (trader)
	local ship = trader.ship
	local stats = ship:GetStats()
	local engine = ship:GetEquip('ENGINE', 0)
	local hyperclass = 0
	if engine == 'DRIVE_CLASS1' then hyperclass = 1
	elseif engine == 'DRIVE_CLASS2' then hyperclass = 2
	elseif engine == 'DRIVE_CLASS3' then hyperclass = 3
	elseif engine == 'DRIVE_CLASS4' then hyperclass = 4
	elseif engine == 'DRIVE_CLASS5' then hyperclass = 5
	elseif engine == 'DRIVE_CLASS6' then hyperclass = 6
	elseif engine == 'DRIVE_CLASS7' then hyperclass = 7
	elseif engine == 'DRIVE_CLASS8' then hyperclass = 8
	elseif engine == 'DRIVE_CLASS9' then hyperclass = 9
	elseif engine == 'DRIVE_MIL1' then hyperclass = 1
	elseif engine == 'DRIVE_MIL2' then hyperclass = 2
	elseif engine == 'DRIVE_MIL3' then hyperclass = 3
	elseif engine == 'DRIVE_MIL4' then hyperclass = 4 end
	local fuel = math.ceil(hyperclass*hyperclass * max_trader_pick_dist / stats.maxHyperspaceRange)

	ship:AddEquip('HYDROGEN', fuel)
end

local _pick_dest = function (trader)
	local nearbysystems = Game.system:GetNearbySystems(max_trader_pick_dist,
		function (s)
			return ((#s:GetStationPaths() > 0) and trader.ship:CanHyperspaceTo(s.path) == 'OK')
		end)
	if #nearbysystems == 0 then
		print(trader.shipname .. " nowhere to jump")
		trader.dest = nil
		return
	end
	local nearbysystem = nearbysystems[Engine.rand:Integer(1,#nearbysystems)]

	trader.dest = nearbysystem.path
end

local _equip_trader = function (trader)
	local shiptype = ShipType.GetShipType(trader.shipname)
	local default_drive = shiptype.defaultHyperdrive

	trader.ship:AddEquip(default_drive)
	trader.ship:AddEquip('CARGO_LIFE_SUPPORT')
	trader.ship:AddEquip('SHIELD_GENERATOR', Engine.rand:Integer(1, 2))
end

local _set_timer = function (trader)
	if trader.state == 'DOCKED' then
		Timer:CallAt(trader.due, function ()
			if trader.ship and trader.ship:exists() then
				trader.state = 'INBOUND'
				trader.ship:Undock()
			end
		end)
	elseif trader.state == 'PARKED' then
		local station = _random_station()
		if station == nil then return end

		Timer:CallAt(trader.due, function ()
			if trader.ship and trader.ship:exists() then
				trader.ship:AIDockWith(station)
			end
		end)
	elseif trader.state == 'CLOUD' then
		local station = _random_station()
		if station == nil then return end

		Timer:CallAt(trader.due, function ()
			if trader.ship and trader.ship:exists() then
				_equip_trader(trader)
				_add_items(trader, 1)
				trader.state = 'INBOUND'
				trader.ship:AIDockWith(station)
			end
		end)
	end
end

local _insert_trader = function (ship, shipname, state, due)
	local trader = {
		ship		= ship,
		shipname	= shipname,
		state		= state, -- 'INBOUND', 'DOCKED', 'HYPERSPACE', 'FLEEING', 'COWERING', 'CLOUD', 'PARKED'
		due		= due,
		dest		= nil,
	}

	traders_count = traders_count + 1
	print("inserting trader, traders: " .. traders_count)
	traders[ship] = trader

	return trader
end

local _add_trader = function (state)
	local shiptypes = ShipType.GetShipTypes('SHIP', function (t) return t.hullMass >= min_trader_hull_mass end)
	if #shiptypes == 0 then return end
	local station = _random_station()
	local shipname = shiptypes[Engine.rand:Integer(1,#shiptypes)]
	local due = Game.time + Engine.rand:Integer(min_wait, max_wait)

	print("adding " .. state .. " trader")

	if state == 'DOCKED' then
		local state = state
		local ship = Space.SpawnShipDocked(shipname, station)
		if ship == nil then
			ship = Space.SpawnShipParked(shipname, station)
			state = 'PARKED'
		end
		if ship == nil then
			return
		end

		local trader = _insert_trader(ship, shipname, state, due)
		_equip_trader(trader)
		_refuel(trader)
		_add_items(trader, -1)
		_pick_dest(trader)
		_set_timer(trader)
	elseif state == 'CLOUD' then
		local nearbysystems = Game.system:GetNearbySystems(max_trader_dist,
			function (s)
				return (#s:GetStationPaths() > 0)
			end)
		if #nearbysystems == 0 then return end
		local nearbysystem = nearbysystems[Engine.rand:Integer(1,#nearbysystems)]

		local ship = Space.SpawnShip(shipname, 8, 14, {nearbysystem.path, due})
		local trader = _insert_trader(ship, shipname, state, due)

		_set_timer(trader)
	elseif state == 'INBOUND' then
		local ship = Space.SpawnShip(shipname, 3, 10)
		local trader = _insert_trader(ship, shipname, state, due)

		_equip_trader(trader)
		_add_items(trader, 1)
		ship:AIDockWith(station)
	end
end

local _remove_trader = function (ref)
	traders_count = traders_count - 1
	print("removing trader, traders: " .. traders_count)
	traders[ref] = nil
end

local _periodic_call
_periodic_call = function ()
	print("periodic called")
	local system = Game.system
	local population = system.population
	if population == 0 then return end
	local lawlessness = system.lawlessness
	local due = Game.time + Engine.rand:Integer(min_wait, max_wait) * (lawlessness*Engine.rand:Integer(1, 20) + 1) / population

	Timer:CallAt(due, function ()
		if disable_periodic == 0 then
			_add_trader('CLOUD')
			_periodic_call()
		end
	end)
end


local onShipHit = function (ship, attacker)
	if ship:IsPlayer() then return end

	for ref, trader in pairs(traders) do
		if ship == trader.ship then
			local int = Engine.rand:Integer(1, 3)

			trader.state = 'FLEEING'
			if int == 1 then
				local equiplist = ship:GetEquip('CARGO')

				if #equiplist == 0 then return end

				ship:Jettison(equiplist[Engine.rand:Integer(1, #equiplist)])
			elseif int == 2 then
				_pick_dest(trader)
				_hyperspace(trader)
			elseif int == 3 then
				ship:AIDockWith(_random_station())
			end
			return
		end
	end
end

local onShipDestroyed = function (ship, body)
	for ref, trader in pairs(traders) do
		if ship == trader.ship then
			_remove_trader(ref)
			return
		end
	end
end

local onShipDocked = function (ship, station)
	if ship:IsPlayer() then return end

	for ref, trader in pairs(traders) do
		if ship == trader.ship then
			if trader.state == 'FLEEING' then
				trader.state = 'COWERING' -- just waits forever
				print(ship.label .. " cowering")
			else
				local due = Game.time + Engine.rand:Integer(min_wait, max_wait)

				trader.due = due
				trader.state = 'DOCKED'
				print(ship.label .. " docked")
				_remove_items(trader)
				_refuel(trader)
				_add_items(trader, -1)
				_pick_dest(trader)
				_set_timer(trader)
			end
			return
		end
	end
end

local onShipUndocked = function (ship, station)
	if ship:IsPlayer() then return end

	for ref, trader in pairs(traders) do
		if trader.ship == ship then
			if trader.dest then
				if _hyperspace(trader) then
					print(ship.label .. " failed to hyperspace")
				else
					return
				end
			else
				print(ship.label .. " no hyperspace destination")
			end
			ship:AIFlyTo(_random_station())
			return
		end
	end
end

local _populate_system = function ()
	local population = Game.system.population
	if population == 0 then return end -- no point trading with an empty system
	local lawlessness = Game.system.lawlessness

	--[[
	traders will be attracted by:
	 - large populations (more people to sell your shit to)
	 - lots of imports (easy to source elsewhere)
	 - lots of exports (easy to sell elsewhere)

	traders will be put off by:
	 - high lawlessness (getting all shot up is bad for business)
	 - small populations (less people to buy stuff)
	]]

	-- start with one ship per half-billion population
	local num_trade_ships = population * traders_per_population

	-- reduce based on lawlessness
	num_trade_ships = num_trade_ships * (1-lawlessness)

	local base_price_alterations = Game.system:GetCommodityBasePriceAlterations()

	local imports = 0
	local exports = 0

	for k,v in pairs(base_price_alterations) do
		if v > 10 then
			imports = imports+2
		elseif v > 2 then
			imports = imports+1
		elseif v < -10 then
			exports = exports+2
		elseif v < -2 then
			exports = exports+1
		end
	end

	for i = 0, num_trade_ships, 1 do
		-- 80% chance of spawning this ship. this is somewhat arbitrary,
		-- but it does mean the player can't assume that system x will
		-- always have n trade ships
		if Engine.rand:Number(1) <= 0.8 then

			local spawn_in_starport = false
			
			if exports > imports then
				if Engine.rand:Number(exports) > imports then
					spawn_in_starport = true
				end
			elseif exports < imports then
				if Engine.rand:Number(imports) <= exports then
					spawn_in_starport = true
				end
			else
				if Engine.rand:Number(1) <= 0.5 then
					spawn_in_starport = true
				end
			end

			if spawn_in_starport then
				_add_trader('DOCKED')
			else
				_add_trader('INBOUND')
			end

		end
	end

end

local onLeaveSystem = function (ship)
	if not ship:IsPlayer() then return end

	disable_periodic = 1
	for ref, trader in pairs(traders) do
		if not trader.state == 'HYPERSPACE' then
			_remove_trader(ref)
		end
	end
end

local onEnterSystem = function (ship)
	if not ship:IsPlayer() then return end

	for ref, trader in pairs(traders) do
		if trader.state == 'HYPERSPACE' and trader.dest and
		   trader.dest:IsSameSystem(Game.system.path) then
			trader.dest = nil
			trader.state = 'INBOUND'
		else
			_remove_trader(ref)
		end
	end

	disable_periodic = 0
	_populate_system()
	_periodic_call()
end

local loaded_data

local onGameStart = function ()
	traders = {}
	traders_count = 0

	if not loaded_data then
		_populate_system()
	end

	_periodic_call()

	loaded_data = nil
end

local serialize = function ()
	return { traders = traders, traders_count = traders_count, disable_periodic = disable_periodic }
end

local unserialize = function (data)
	loaded_data = data

	for k,trader in pairs(loaded_data.traders) do
		_set_timer(trader)
	end
end

EventQueue.onEnterSystem:Connect(onEnterSystem)
EventQueue.onLeaveSystem:Connect(onLeaveSystem)
EventQueue.onGameStart:Connect(onGameStart)
EventQueue.onShipDestroyed:Connect(onShipDestroyed)
EventQueue.onShipHit:Connect(onShipHit)
EventQueue.onShipDocked:Connect(onShipDocked)
EventQueue.onShipUndocked:Connect(onShipUndocked)

Serializer:Register("TradeShips", serialize, unserialize)
