
__pendingEvents = {}
__eventListeners = {}

function EmitEvents()
	print("Lua EmitEvents");
	for i,event in ipairs(__pendingEvents) do
		mods = __eventListeners[event.type]
		if mods then
			for mod,j in pairs(mods) do
				m = _G[mod]
				m[event.type](m, event)
			end
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

function serialize(val)
	local out
-- TODO serialize ObjectWrapper objects
--	assert(type(val) ~= 'userdata')
	if type(val) == 'number' then
		out = 'f' .. val .. '\n'
	elseif type(val) == 'boolean' then
		if val == true then out = 'b1\n' else out = 'b0\n' end
	elseif type(val) == 'string' then
		out = 's' .. #val .. '\n' .. val .. '\n'
	elseif type(val) == 'table' then
		out = 't' .. '\n'
		-- key, value pairs, terminated by 'n\n'
		for k,v in pairs(val) do
			if (type(v) ~= 'function') and
			   (type(k) ~= 'function') then
				out = out .. serialize(k)
				out = out .. serialize(v)
			end
		end
		out = out .. 'n\n'
	end
	return out
end

function unserialize(val, addtotable, start)
	start = start or 1
	if start > #val then
		return nil
	end
	if val:sub(start,start) == 'n' then
		return start+2, nil
	elseif val:sub(start,start) == 'f' then
		local last = string.find(val, '\n', start)
		return last+1, tonumber(val:sub(start+1,last-1))
	elseif val:sub(start,start) == 'b' then
		local v = tonumber(val:sub(start+1,start+1))
		start = start+3
		if v == 0 then return start,false else return start,true end
	elseif val:sub(start,start) == 's' then
		local last = string.find(val, '\n', start)
		local len = tonumber(val:sub(start+1,last-1))
		local v = val:sub(last+1, last+len)
		assert(#v == len)
		return last+len+2,v -- 2 newlines
	elseif val:sub(start,start) == 't' then
		-- table! key, value pairs
		addtotable = addtotable or {}
		start = start + 2
		repeat
			local k,v
			start, k = unserialize(val, nil, start)
			if type(k) ~= 'nil' then
				start, v = unserialize(val, nil, start)
				addtotable[k] = v
			end
		until k == nil
		return start, addtotable
	end
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
		EventListen(self, "onCreateBB")
		EventListen(self, "onUpdateBB")
		self.ads = {}
	end,

	GetPlayerMissions = function(self)
		return { { description="Hello world", client="Mr Morton", reward=1234, status='completed' },
	                 { description="Eat your own head", client="God", reward=500, status='failed' } }
	end,

	Serialize = function(self)
		return serialize(self)
	end,

	Unserialize = function(self, data)
		unserialize(data, self)
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
	end,

	onCreateBB = function(self, args)
		print("Creating bb adverts for " .. args[1]:GetLabel())
		local station = args[1]
		
		table.insert(self.ads, {id=#self.ads+1, bb=station})
		station:SpaceStationAddAdvert(self.__name, #self.ads, "Click me!")

		table.insert(self.ads, {id=#self.ads+1, bb=station})
		station:SpaceStationAddAdvert(self.__name, #self.ads, "Another advert")
	end,

	onUpdateBB = function(self, args)
		-- insert or delete new ads at random
		print("Updating bb adverts for " .. args[1]:GetLabel())
	end,

	DialogHandler = function(self, dialog, ad_ref, optionClicked)
		print("dialog handler for " .. ad_ref .. " clicked " .. optionClicked)
		if optionClicked == -1 then
			dialog:Close()
			self.ads[ad_ref].bb:SpaceStationRemoveAdvert(self.__name, ad_ref)
		else
			dialog:Clear()
			dialog:SetTitle("Hello old beans!")
			dialog:SetMessage("Hello you plenty good chap, blah blah")
			dialog:AddOption("$1", 1);
			dialog:AddOption("$10", 2);
			dialog:AddOption("$100", 3);
			dialog:AddOption("$1000", 4);
			dialog:AddOption("$10000", 5);
			dialog:AddOption("$100000", 6);
			dialog:AddOption("Hang up.", -1);
		end
	end,
}

