
PiModule {
	__name='mymod', 
	x=123,
	
	Init = function(self)
		print('init() mymod')
		print(self.x)
		EventListen(self, "onPlayerChangeTarget")
	end,

	DoSomething = function(self)
		print('hi')
	end,

	onPlayerChangeTarget = function(self)
		print('mymod got onPlayerChangeTarget');
		EventIgnore(self, "onPlayerChangeTarget")
	end
}


