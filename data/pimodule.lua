
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
	elseif type(val) == 'userdata' then
		out = UserDataSerialize(val)
	else
		assert(0)
	end
	return out
end

function unserialize(val, addtotable, start)
	start = start or 1
	if start > #val then
		return start, nil
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
			if k ~= nil then
				start, v = unserialize(val, nil, start)
				addtotable[k] = v
			end
		until k == nil
		return start, addtotable
	elseif val:sub(start,start) == 'o' then
		local last = string.find(val, '\n', start)
		return last+1, UserDataUnserialize(val:sub(start+1, last-1))
	end
end

-- Keep in sync with EquipType.h Equip::Type...
Equip = {
	NONE=0, HYDROGEN=1, LIQUID_OXYGEN=2, METAL_ORE=3, CARBON_ORE=4, METAL_ALLOYS=5,
	PLASTICS=6, FRUIT_AND_VEG=7, ANIMAL_MEAT=8, LIVE_ANIMALS=9, LIQUOR=10, GRAIN=11, TEXTILES=12, FERTILIZER=13,
	WATER=14, MEDICINES=15, CONSUMER_GOODS=16, COMPUTERS=17, ROBOTS=18, PRECIOUS_METALS=19,
	INDUSTRIAL_MACHINERY=20, FARM_MACHINERY=21, MINING_MACHINERY=22, AIR_PROCESSORS=23, SLAVES=24,
	HAND_WEAPONS=25, BATTLE_WEAPONS=26, NERVE_GAS=27, NARCOTICS=28, MILITARY_FUEL=29, RUBBISH=30, RADIOACTIVES=31
}

Module = {}
function Module:new(o)
	o = o or {}
	setmetatable(o, self)
	self.__index = self
	PiModule(o)
	return o
end
function Module:Serialize()
	return serialize(self)
end

function Module:Unserialize(data)
	unserialize(data, self)
end
-- default to performing transaction when clicked (can override to make other
-- nasty stuff happen
function Module:TraderOnClickSell(self, dialog, comType)
	return true
end
function Module:TraderOnClickBuy(self, dialog, comType)
	return true
end

--[[
a = ObjectWrapper:new()
b = Pi.GetPlayer()
print(a:IsBody())
print(b:IsBody())
print(b:GetLabel())
a:print()
b:print()
print(a == b)
--]]
print(Pi.GetPlayer())
