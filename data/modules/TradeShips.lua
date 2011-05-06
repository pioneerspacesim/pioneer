local onEnterSystem = function (player)
	if not player:IsPlayer() then return end

	local stations = Space.GetBodies(function (body) return body:isa("SpaceStation") end)
	if #stations == 0 then return end

	local lawlessness = Game.system.lawlessness
	local population = Game.system.population

	--[[
	traders will be attracted by:
	 - large populations (more people to sell your shit to)
	 - lots of imports (easy to source elsewhere)
	 - lots of exports (easy to sell elsewhere)

	traders will be put off by:
	 - high lawlessness (getting all shot up is bad for business)
	 - small populations (less people to buy stuff)
	]]

	-- no point trading with an empty system
	if population == 0 then
		return
	end

	-- start with one ship per half-billion population
	local num_trade_ships = population*2

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

	local shiptypes = ShipType.GetShipTypes('SHIP', function (t) return t.hullMass >= 100 end)
	if #shiptypes == 0 then return end

	for i = 0, num_trade_ships, 1 do
		-- 80% chance of spawning this ship. this is somewhat arbitrary,
		-- but it does mean the player can't assume that system x will
		-- always have n trade ships
		if Engine.rand:Number() <= 0.8 then

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
				if Engine.rand:Number() <= 0.5 then
					spawn_in_starport = true
				end
			end

			local shiptype = shiptypes[Engine.rand:Integer(1,#shiptypes)]

			local station = stations[Engine.rand:Integer(1,#stations)]

			if spawn_in_starport then
				pcall(function () return Space.SpawnShipDocked(shiptype, station) end)
			else
				-- XXX random the due time a bit so that some aren't in system yet
				local ship = Space.SpawnShip(shiptype, 3, 8)
				ship:AIDockWith(station)
			end

		end
	end
end

EventQueue.onEnterSystem:Connect(onEnterSystem)
