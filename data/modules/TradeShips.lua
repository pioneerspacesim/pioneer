local onEnterSystem = function (sys, player)
	local lawlessness = sys:GetLawlessness()
	local population = sys:GetPopulation()

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

	local base_price_alterations = sys:GetCommodityBasePriceAlterations()

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
		if Pi.rand:Real(0,1) <= 0.8 then

			local spawn_in_starport = false
			
			if exports > imports then
				if Pi.rand:Real(0,exports) > imports then
					spawn_in_starport = true
				end
			elseif exports < imports then
				if Pi.rand:Real(0,imports) <= exports then
					spawn_in_starport = true
				end
			else
				if Pi.rand:Real(0,1) <= 0.5 then
					spawn_in_starport = true
				end
			end

			local starport = sys:GetRandomStarport()
			if not starport then
				-- not much for traders to do if there's no starports
				return
			end
			local body = sys:GetBody(starport)

			if spawn_in_starport then
				local ship, e = Pi.SpawnRandomDockedShip(body, 10, 100, 10000000)
			else
				-- XXX random the due time a bit so that some aren't in system yet
				ship, e = Pi.SpawnRandomShip(0, 10, 100, 10000000)
				if ship then
					ship:AIDoDock(body)
				end
			end

		end
	end
end

EventQueue.onEnterSystem:Connect(onEnterSystem)
