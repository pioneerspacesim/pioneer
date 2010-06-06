
-- Rename some wrapped classes
Object = ObjectWrapper
StarSystem = SysLoc
SBody = SBodyPath

__pendingEvents = {}
__eventListeners = {}

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

function EmitEvents()
	print("Lua EmitEvents");
	for i,event in ipairs(__pendingEvents) do
		print(event.type)
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

-- Keep in sync with EquipType.h Equip::Type...
Equip = {
	NONE=0, HYDROGEN=1, LIQUID_OXYGEN=2, METAL_ORE=3, CARBON_ORE=4, METAL_ALLOYS=5,
	PLASTICS=6, FRUIT_AND_VEG=7, ANIMAL_MEAT=8, LIVE_ANIMALS=9, LIQUOR=10, GRAIN=11, TEXTILES=12, FERTILIZER=13,
	WATER=14, MEDICINES=15, CONSUMER_GOODS=16, COMPUTERS=17, ROBOTS=18, PRECIOUS_METALS=19,
	INDUSTRIAL_MACHINERY=20, FARM_MACHINERY=21, MINING_MACHINERY=22, AIR_PROCESSORS=23, SLAVES=24,
	HAND_WEAPONS=25, BATTLE_WEAPONS=26, NERVE_GAS=27, NARCOTICS=28, MILITARY_FUEL=29, RUBBISH=30, RADIOACTIVES=31,

	MISSILE_UNGUIDED=32, MISSILE_GUIDED=33, MISSILE_SMART=34, MISSILE_NAVAL=35, ATMOSPHERIC_SHIELDING=36, ECM_BASIC=37, SCANNER=38, ECM_ADVANCED=39, SHIELD_GENERATOR=40,
	LASER_COOLING_BOOSTER=41, CARGO_LIFE_SUPPORT=42, AUTOPILOT=43,
	RADAR_MAPPER=44, FUEL_SCOOP=45, HYPERCLOUD_ANALYZER=46, HULL_AUTOREPAIR=47, SHIELD_ENERGY_BOOSTER=48,
	DRIVE_CLASS1=49, DRIVE_CLASS2=50, DRIVE_CLASS3=51, DRIVE_CLASS4=52, DRIVE_CLASS5=53, DRIVE_CLASS6=54, DRIVE_CLASS7=55,
	DRIVE_MIL1=56, DRIVE_MIL2=57, DRIVE_MIL3=58, DRIVE_MIL4=59,
	PULSECANNON_1MW=60, PULSECANNON_DUAL_1MW=61, PULSECANNON_2MW=62, PULSECANNON_RAPID_2MW=63, PULSECANNON_4MW=64, PULSECANNON_10MW=65, PULSECANNON_20MW=66,
	MININGCANNON_17MW=67, SMALL_PLASMA_ACCEL=68, LARGE_PLASMA_ACCEL=69,
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
