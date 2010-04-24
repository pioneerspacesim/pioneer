
__pendingEvents = {}
__eventListeners = {}

function EmitEvents()
	print("Lua EmitEvents");
	for i,event in ipairs(__pendingEvents) do
		mods = __eventListeners[event.type]
		for mod,j in pairs(mods) do
			m = _G[mod]
			m[event.type](m, event)
		end
	end
	__pendingEvents = {}
end

function EventListen(mod, event)
	if __eventListeners[event] == nil then
		__eventListeners[event] = {}
	end
	__eventListeners[event][mod.__name] = true
end

function EventIgnore(mod, event)
	__eventListeners[event][mod.__name] = nil
end

---[[
a = ObjectWrapper:new()
b = PiPlayer()
print(a:IsBody())
print(b:IsBody())
print(b:GetLabel())
a:print()
b:print()
print(a == b)
--]]
PiModule {
	__name='mymod', 
	x=123,
	
	Init = function(self)
		print('init() mymod')
		print(self.x)
		EventListen(self, "onPlayerChangeTarget")
		EventListen(self, "onShipKilled")
	end,

	DoSomething = function(self)
		print('hi')
	end,

	onPlayerChangeTarget = function(self)
		print('mymod got onPlayerChangeTarget');
		EventIgnore(self, "onPlayerChangeTarget")
	end,

	onShipKilled = function(self, args)
		print(args["type"])
		for i,j in pairs(args) do print(i,j) end
		print(args[1]:GetLabel() .. " was killed by " ..  args[2]:GetLabel())
		collectgarbage("collect")
	end
}

