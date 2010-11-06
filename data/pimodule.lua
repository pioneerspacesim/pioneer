
-- Rename some wrapped classes
Object = ObjectWrapper
StarSystem = SysLoc
SBody = SBodyPath

-- A 'fake module' to dispatch object events to modules that only care
-- about the event for a particular object (like listening for ship X being attacked)
ObjectEventDispatcher = {
	__onShipAttackedListeners = {},
	onShipAttacked = function(self, args)
		local listeners = self.__onShipAttackedListeners[args[1]]
		if listeners ~= nil then
			for mod,j in pairs(listeners) do
				local m = _G[mod]
				m[j](m, args)
			end
		end
	end,
}

Object.OnShipAttacked = function(ship, mod, methodName)
	if methodName == nil then
		ObjectEventDispatcher.__onShipAttackedListeners[ship][mod.__name] = nil
	else
		if ObjectEventDispatcher.__onShipAttackedListeners[ship] == nil then
       			ObjectEventDispatcher.__onShipAttackedListeners[ship] = {}
		end
		ObjectEventDispatcher.__onShipAttackedListeners[ship][mod.__name] = methodName
	end
end

-- other jizz

Pi.rand = Rand:new(os.time())

-- Some very useful utility functions --------------------

function _(str, bits)
	-- str = gettext(str)
	s, num = string.gsub(str, '%%([0-9]+)', function(w) return bits[tonumber(w)] end)
	return s
end

function format_money(amount)
	return string.format('$%.1f', amount)
end

-- Bits that make modules work ---------------------------

-- Start with the 'fake module' ObjectEventDispatcher listening for some events
__pendingEvents = {}
__eventListeners = { onShipAttacked = {ObjectEventDispatcher=true} }

function EmitEvents()
	for i,event in ipairs(__pendingEvents) do
		local mods = __eventListeners[event.type]
		if mods then
			for mod,j in pairs(mods) do
				local m = _G[mod]
				m[event.type](m, event)
			end
		end
	end
	__pendingEvents = {}
end

__piTimers = {}

function UpdateOncePerRealtimeSecond()
	local t = Pi.GetGameTime()
	for i,timer in pairs(__piTimers) do
		if (t >= timer.time) then
			timer.func(timer.args)
			__piTimers[i] = nil
		end
	end
end

Pi.AddTimer = function(time, fn, args)
	table.insert(__piTimers, {time=time, func=fn, args=args})
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
		local udata = UserDataSerialize(val)
		out = 'o' .. #udata .. '\n' .. udata
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
		local len = tonumber(val:sub(start+1, last-1))
		return last+len+1, UserDataUnserialize(val:sub(last+1, last+len))
	end
end

dofile "data/pienums.lua"

-- Keep in sync with Polit.h enum Crime
Pi.Crime = {
	CRIME_TRADING_ILLEGAL_GOODS=1,
	CRIME_WEAPON_DISCHARGE=2,
	CRIME_PIRACY=4,
	CRIME_MURDER=8,
}

Module = {}
function Module:new(o)
	o = o or {}
	setmetatable(o, self)
	self.__index = self
	PiModule(o)
	return o
end
function Module:EventListen(event)
	if __eventListeners[event] == nil then
		__eventListeners[event] = {}
	end
	__eventListeners[event][self.__name] = true
end

function Module:EventIgnore(event)
	__eventListeners[event][self.__name] = nil
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

