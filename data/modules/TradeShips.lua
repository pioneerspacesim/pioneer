Module:new {
	__name = 'TradeShips',

	Init = function(self)
		self:EventListen("onEnterSystem")
	end,

	onEnterSystem = function(self)
		local sys = Pi.GetCurrentSystem()
		local lawlessness = sys:GetSystemLawlessness()
		local population = sys:GetSystemPopulation()

		print(string.format("TradeShips: entering system: %s [lawless %f pop %f]", sys:GetSystemName(), lawlessness, population))

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
			print("TradeShips: nobody to trade with here :(")
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

		print("TradeShips: num ships: "..num_trade_ships)
		print(string.format("TradeShips: imports %d exports %d", imports, exports))

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

				-- XXX spawn the ship

			end
		end

	end,
}
