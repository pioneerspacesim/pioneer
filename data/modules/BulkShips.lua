Module:new {
	__name = 'BulkShips',

	Init = function(self)
		self:EventListen("onEnterSystem")
	end,

	onEnterSystem = function(self)
		local sys = Pi.GetCurrentSystem()
		local starport = sys:GetRandomStarport(Pi.rand)

		print(string.format("spawning static ship near %s in system %s", starport:GetBodyName(), sys:GetSystemName()))

		local ship, e = Pi.SpawnRandomStaticShip(Pi.FindBodyForSBody(starport))
		if e then
			print("BulkShips: "..e)
		end
	end,
}
