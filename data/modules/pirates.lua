
Module:new {
	__name = 'Pirates',

	Init = function(self)
		self:EventListen("onEnterSystem")
	end,

	onEnterSystem = function(self)
		--print("Pirate module onEnterSystem")
		local plvl = Pi.GetCurrentSystem():get_lawlessness()
		--print("Lawlessness " .. plvl)

		local max_pirates = 6
		while max_pirates > 0 and Pi.rand:Real(0,1) < plvl do
			max_pirates = max_pirates-1

			local power = Pi.rand:Real(0, plvl)
			local minMass = 10
			local maxMass = 50 + 150*plvl
			ship, e = Pi.SpawnRandomShip(Pi.GetGameTime(), power, minMass, maxMass)
			if not e then
				--print("Pirates module spawned " .. ship:get_label() .. " with power " .. power)
				ship:ai_do_kill(Pi.GetPlayer())
			--else
				--print("Pirate module: " .. e)
			end
		end
	end,
}

