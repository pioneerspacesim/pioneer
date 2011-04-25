local ships
local candidate_ships

local onGameStart = function ()
	ships = ShipType.GetShipTypes(ShipType.Tag.STATIC_SHIP)

	candidate_ships = {}
	for n,t in pairs(ships) do
		table.insert(candidate_ships, n)
	end
end

local onEnterSystem = function (player)
	if #candidate_ships == 0 then
		return
	end

	local population = Game.system:GetPopulation()

	if population == 0 then
		return
	end

	local stations = Space.GetSpaceStations()
	if #stations == 0 then
		return
	end

	--[[
	assuming these are huge supply ships and not your run-of-the-mill
	traders and not warships or whatever, we'll do it like this:

	- first one is free
	- one ship per billion up to 4 billion
	- one ship per 5 billion after that
	]]

	local num_bulk_ships = 1;
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
        local ship = Space.SpawnShipParked(candidate_ships[Engine.rand:Integer(1,#candidate_ships)], station)
	end
end

EventQueue.onGameStart:Connect(onGameStart)
EventQueue.onEnterSystem:Connect(onEnterSystem)
