local onEnterSystem = function (sys, player)
	local population = sys:get_population()

	if population == 0 then
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
		local starport = sys:get_random_starport()
		if not starport then return end

		local ship, e = Pi.SpawnRandomStaticShip(sys:get_body(starport))
		if e then
			print("BulkShips: "..e)
		end
	end
end

Module:new {
	__name = 'BulkShips',

	Init = function(self)
		EventQueue.onEnterSystem:connect(onEnterSystem)
	end,
}
