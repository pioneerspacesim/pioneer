Module:new {
	__name = 'TradeShips',

	Init = function(self)
		self:EventListen("onEnterSystem")
	end,

	onEnterSystem = function(self)
		print("Traders module onEnterSystem")
	end,
}
