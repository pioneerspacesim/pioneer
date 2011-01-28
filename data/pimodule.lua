
-- Rename some wrapped classes
Object = ObjectWrapper
StarSystem = SysLoc
SBody = SBodyPath

-- A 'fake module' to dispatch object events to modules that only care
-- about the event for a particular object (like listening for ship X being attacked)
ObjectEventDispatcher = {
	__onShipAttackedListeners = {},
	__onShipKilledListeners = {},
	onShipAttacked = function(self, args)
		local listeners = self.__onShipAttackedListeners[args[1]]
		if listeners ~= nil then
			for mod,j in pairs(listeners) do
				local m = _G[mod]
				m[j](m, args)
			end
		end
	end,
	onShipKilled = function(self, args)
		local listeners = self.__onShipKilledListeners[args[1]]
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
		ObjectEventDispatcher.__onShipAttackedListeners[ship] = 
			ObjectEventDispatcher.__onShipAttackedListeners[ship] or {}
		ObjectEventDispatcher.__onShipAttackedListeners[ship][mod.__name] = methodName
	end
end
Object.OnShipKilled = function(ship, mod, methodName)
	if methodName == nil then
		ObjectEventDispatcher.__onShipKilledListeners[ship][mod.__name] = nil
	else
		ObjectEventDispatcher.__onShipKilledListeners[ship] = 
			ObjectEventDispatcher.__onShipKilledListeners[ship] or {}
		ObjectEventDispatcher.__onShipKilledListeners[ship][mod.__name] = methodName
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

Pi.RandomShipRegId = function()
	local letters = "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
	local a = Pi.rand:Int(1, #letters)
	local b = Pi.rand:Int(1, #letters)
	return string.format("%s%s-%04d", letters:sub(a,a), letters:sub(b,b), Pi.rand:Int(0, 9999))
end
 
-- Bits that make modules work ---------------------------

-- Start with the 'fake module' ObjectEventDispatcher listening for some events
__pendingEvents = {}
__eventListeners = {
	onShipAttacked = {ObjectEventDispatcher=true},
	onShipKilled = {ObjectEventDispatcher=true}
}

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

-- The following enums haven't been put in pienums because
-- pienums is also included from the pioneer model lua context,
-- which doesn't have the Pi & SBody objects.

-- Keep in sync with Polit.h enum Crime
Pi.Crime = {
	CRIME_TRADING_ILLEGAL_GOODS=1,
	CRIME_WEAPON_DISCHARGE=2,
	CRIME_PIRACY=4,
	CRIME_MURDER=8,
}

-- Keep in sync with StarSystem.h SBody enum BodyType
SBody.TYPE_GRAVPOINT = 0;
SBody.TYPE_BROWN_DWARF = 1;
SBody.TYPE_STAR_M = 2;
SBody.TYPE_STAR_K = 3;
SBody.TYPE_STAR_G = 4;
SBody.TYPE_STAR_F = 5;
SBody.TYPE_STAR_A = 6;
SBody.TYPE_STAR_B = 7;
SBody.TYPE_STAR_O = 8;
SBody.TYPE_STAR_M_GIANT = 9;
SBody.TYPE_WHITE_DWARF = 10;
SBody.TYPE_PLANET_SMALL_GAS_GIANT = 11;
SBody.TYPE_PLANET_MEDIUM_GAS_GIANT = 12;
SBody.TYPE_PLANET_LARGE_GAS_GIANT = 13;
SBody.TYPE_PLANET_VERY_LARGE_GAS_GIANT = 14;
SBody.TYPE_PLANET_ASTEROID = 15;
SBody.TYPE_PLANET_LARGE_ASTEROID = 16;
SBody.TYPE_PLANET_DWARF = 17;
SBody.TYPE_PLANET_SMALL = 18;
SBody.TYPE_PLANET_WATER = 19;
SBody.TYPE_PLANET_DESERT = 20;
SBody.TYPE_PLANET_CO2 = 21;
SBody.TYPE_PLANET_METHANE = 22;
SBody.TYPE_PLANET_WATER_THICK_ATMOS = 23;
SBody.TYPE_PLANET_CO2_THICK_ATMOS = 24;
SBody.TYPE_PLANET_METHANE_THICK_ATMOS = 25;
SBody.TYPE_PLANET_HIGHLY_VOLCANIC = 26;
SBody.TYPE_PLANET_INDIGENOUS_LIFE = 27;
SBody.TYPE_PLANET_TERRAFORMED_POOR = 28;
SBody.TYPE_PLANET_TERRAFORMED_GOOD = 29;
SBody.TYPE_STARPORT_ORBITAL = 30;
SBody.TYPE_STARPORT_SURFACE = 31;
SBody.TYPE_MAX = 32;
		
-- Keep in sync with StarSystem.h SBody enum BodySuperType
SBody.SUPERTYPE_NONE = 0;
SBody.SUPERTYPE_STAR = 1;
SBody.SUPERTYPE_ROCKY_PLANET = 2;
SBody.SUPERTYPE_GAS_GIANT = 3;
SBody.SUPERTYPE_STARPORT = 4;

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

