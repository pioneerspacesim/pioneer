local loaded

local spawnShips = function ()
	local population = Game.system.population

	if population == 0 then
		return
	end

	local stations = Space.GetBodies(function (body) return body:isa("SpaceStation") end)
	if #stations == 0 then
		return
	end

	local shiptypes = ShipType.GetShipTypes('STATIC_SHIP')
	if #shiptypes == 0 then return end

	--[[
	assuming these are huge supply ships and not your run-of-the-mill
	traders and not warships or whatever, we'll do it like this:

	- first one is free
	- one ship per billion up to 4 billion
	- one ship per 5 billion after that
	]]

	local num_bulk_ships = 1
	while population > 1 do
		if num_bulk_ships < 4 then
			population = population-1
			num_bulk_ships = num_bulk_ships+1
		elseif population > 5 then
			population = population-5
			num_bulk_ships = num_bulk_ships+1
		else
			break
		end
	end

	for i=1, num_bulk_ships do
	local station = stations[Engine.rand:Integer(1,#stations)]
		Space.SpawnShipParked(shiptypes[Engine.rand:Integer(1,#shiptypes)], station)
	end
end

local onEnterSystem = function (player)
	if not player:IsPlayer() then return end

	spawnShips()
end

local onGameStart = function ()
	if loaded == nil then
		spawnShips()
	end
	loaded = nil
end

local serialize = function ()
	return true
end

local unserialize = function (data)
	loaded = true
end

EventQueue.onEnterSystem:Connect(onEnterSystem)
EventQueue.onGameStart:Connect(onGameStart)

Serializer:Register("BulkShips", serialize, unserialize)
