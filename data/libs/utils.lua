-- Copyright © 2008-2024 Pioneer Developers. See AUTHORS.txt for details
-- Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

local Engine = require 'Engine' -- rand

--
-- Interface: utils
--
-- Collection of utility functions to ease working with tables as data
-- containers or class-like objects.
--
local utils = {}

--
-- Function: numbered_keys
--
-- Transform an iterator to one that returns a numeric index instead of keys
--
-- Example:
--   > for i, v in numbered_keys(pairs(table)) do ... end
--
function utils.numbered_keys(step, context, position)
  local k = position
  local f = function(s, i)
	local v
	k,v = step(s, k)
	if k ~= nil then
	  return (i+1), v
	end
  end
  return f, context, 0
end

--
-- Function: filter
--
-- Transform an iterator to one that only returns items that match some predicate.
--
-- Example:
--   > for k,v in filter(function (k,v) ... return true end, pairs(table))
--
function utils.filter(predicate, step, context, position)
  local f = function (s, k)
	local v
	repeat k,v = step(s,k); until (k == nil) or predicate(k,v)
	return k,v
  end
  return f, context, position
end

--
-- Function: map
--
-- Transform an iterator to one that returns modified keys/values
--
-- Example:
--   > for k,v in map(function (k,v) ... return newk, newv end, pairs(table))
--
function utils.map(transformer, step, context, position)
  local f = function (s, k)
	local v
	k, v = step(s, k)
	if k ~= nil then
	  return transformer(k,v)
	end
  end
  return f, context, position
end

--
-- Function: build_array
--
-- Return an array table containing the values returned by an iterator.
--
-- Example:
--   > array = build_array(pairs(table))
--
function utils.build_array(f, s, k)
  local v
  local t = {}
  while true do
	k, v = f(s, k)
	if k == nil then break end
	table.insert(t, v)
  end
  return t
end

--
-- Function: build_table
--
-- Return a table containing key-value pairs returned by the provided iterator.
--
-- Example:
--   > filtered = build_table(filter(function () ... end, pairs(table)))
--
function utils.build_table(f, s, k)
  local v
  local t = {}
  while true do
	k, v = f(s, k)
	if k == nil then break end
	t[k] = v
  end
  return t
end

--
-- Function: map_table
--
-- Return a new table created from applying a transformer predicate to the
-- values of the provided table object. Key iteration order is undefined
-- (the function will use pairs() internally).
--
-- Example:
--   > transformed = utils.map_table(t, function(k, v) return k .. "1", v end)
--
---@generic K, V, K2, V2
---@param table table<K, V>
---@param predicate fun(k: K, v: V): K2, V2
---@return table<K2, V2>
function utils.map_table(table, predicate)
	local t = {}
	for k, v in pairs(table) do
		k, v = predicate(k, v)
		t[k] = v
	end
	return t
end

--
-- Function: map_array
--
-- Return a new array created from applying a transformer predicate to the
-- values of the provided array object. If the predicate returns a nil value,
-- that value will be skipped.
--
-- Example:
--   > transformed = utils.map_array(t, function(v) return v + 32 end)
--
---@generic T, T2
---@param array T[]
---@param predicate fun(v: T): T2
---@return T2[]
function utils.map_array(array, predicate)
	local t = {}
	for i, v in ipairs(array) do
		v = predicate(v)
		if v ~= nil then table.insert(t, v) end
	end
	return t
end

--
-- Function: filter_table
--
-- Return a new table created from applying a filter predicate to the values
-- of the provided table object. Key iteration order is undefined (the function
-- will use pairs() internally).
--
-- Example:
--   > filtered = utils.filter_table(t, function (k, v) return true end)
--
---@generic K, V
---@param table table<K, V>
---@param predicate fun(k: K, v: V): boolean
function utils.filter_table(table, predicate)
	local t = {}
	for k, v in pairs(table) do
		if predicate(k, v) then t[k] = v end
	end
	return t
end

--
-- Function: filter_array
--
-- Return a new array created from applying a filter predicate to the values
-- of the provided array object.
--
-- Example:
--   > filtered = utils.filter_array(t, function (i, v) return true end)
--
---@generic T
---@param array T[]
---@param predicate fun(v: T): boolean
function utils.filter_array(array, predicate)
	local t = {}
	for _, v in ipairs(array) do
		if predicate(v) then table.insert(t, v) end
	end
	return t
end

-- Function: to_array
--
-- Filters the values of the given table and converts them to an array.
-- Key iteration order is undefined (uses pairs() internally).
---@generic K, V
---@param t table<K, V>
---@param predicate fun(v: V): boolean
---@return V[]
utils.to_array = function(t, predicate)
	local out = {}
	for _, v in pairs(t) do
		if predicate(v) then table.insert(out, v) end
	end
	return out
end

--
-- Function: stable_sort
--
-- Sort the given table in-place and return it. Sort isn't fast but will be
-- stable (default Lua table.sort is fast and unstable).
--
-- stable_sort uses a Merge sort algorithm.
--
-- Example:
--   > sorted_table = stable_sort(unsorted_table, function (a,b) return a < b end)
--
function utils.stable_sort(values, cmp)
	if not cmp then
		cmp = function (a,b) return a <= b end
	end

	local split = function (values)
	   local a = {}
	   local b = {}
	   local len = #values
	   local mid = math.floor(len/2)
	   for i = 1, mid do
		  a[i] = values[i]
	   end
	   for i = mid+1, len do
		  b[i-mid] = values[i]
	   end
	   return a,b
	end

	local merge = function (a,b)
	   local result = {}
	   local a_len = #(a)
	   local b_len = #(b)
	   local i1 = 1
	   local i2 = 1
	   for j = 1, a_len+b_len do
		  if i2 > b_len
			 or (i1 <= a_len and cmp(a[i1], b[i2]))
		  then
			 result[j] = a[i1]
			 i1 = i1 + 1
		  else
			 result[j] = b[i2]
			 i2 = i2 + 1
		  end
	   end
	   return result
	end

	local function merge_sort (values)
	   if #values > 1 then
		  local a, b = split(values)
		  a = merge_sort(a)
		  b = merge_sort(b)
		  values = merge(a, b)
	   end
	   return values
	end

	return merge_sort(values)
end

local __automagic = {
	__index = function(t, k)
		local nt = {}
		t[k] = nt
		return nt
	end
}

--
-- Function: automagic
--
-- Returns an "automagic" table - the table will automatically create a new
-- subtable if accessed via a key that doesn't yet exist in the table.
--
-- Created subtables will be "regular" Lua tables with no automagic capability.
-- To check if an index exists in the table, `rawget()` must be used.
--
-- Example:
-- > local t = utils.automagic()
-- > t.someIndexNotPresent.value = "no error will be thrown!"
--
function utils.automagic(t)
	return setmetatable(t or {}, __automagic)
end

--
-- Class: utils.object
--
-- Default base class for lua classes created with <utils.inherits> or
-- <utils.class>. Implements default serialization and construction methods.
--
local object = {}

object.meta = { __index = object, class="object" }

--
-- Function: New
--
-- Creates and returns a new instance of the default object class.
-- This is overridden in all subclasses.
--
function object.New(args)
	local newinst = {}
	setmetatable( newinst, object.meta )
	return newinst
end

--
-- Function: Serialize
--
-- Method handler called when an instance of this class is about to be
-- serialized to disk. It should perform any needed data transformation, and
-- may return an entirely different object if needed.
--
-- Returning a different object may have limitations if the instance to be
-- serialized contains cyclic references to itself.
--
function object:Serialize()
	return self
end

--
-- Function: Unserialize
--
-- Method handler called when an instance of this class is about to be
-- deserialized from disk. It should perform any needed data transformation,
-- set the new instance's metatable, and may return an entirely different
-- object if needed.
--
-- Returning a different object may have limitations if the serialized instance
-- contains cyclic references to itself.
--
function object.Unserialize(data)
	setmetatable(data, object.meta)
	return data
end

--
-- Interface: utils
--

--
-- Function: inherits
--
-- Returns a new class that implements optional single-inheritance from the
-- provided base class.
--
-- If no base class is provided, the new class will inherit from <utils.object>
-- to provide stub implementations for serialization.
--
-- The returned class has a default constructor defined as `class.New()`.
-- To override the constructor, redefine the Class.New() method and save the
-- `class.New()` method to be called in your custom constructor.
--
utils.inherits = function (baseClass, name)
	local new_class = {}
	local base_class = baseClass or object
	new_class.meta = { __index = new_class, class=name }

	-- generic constructor
	function new_class.New(...)
		local newinst = base_class.New(...)
		setmetatable( newinst, new_class.meta )
		return newinst
	end

	setmetatable( new_class, { __index = base_class } )

	-- Return the class object of the instance
	function new_class:Class()
		return new_class
	end

	function new_class.Unserialize(data)
		local tmp = base_class.Unserialize(data)
		setmetatable(tmp, new_class.meta)
		return tmp
	end

	-- Return the super class object of the instance
	function new_class.Super()
		return base_class
	end

	return new_class
end

--
-- Function: class
--
-- Wrapper for utils.inherits that manages creating new class instances and
-- calling the constructor.
--
utils.class = function (name, base_class)
	base_class = base_class or object
	local new_class = utils.inherits(base_class, name)

	new_class.New = function(...)
		local instance = setmetatable( {}, new_class.meta )

		new_class.Constructor(instance, ...)

		return instance
	end

	new_class.Constructor = function(self, ...)
		if base_class.Constructor then
			base_class.Constructor(self, ...)
		end
	end

	return new_class
end

local _proto = {}

_proto.__clone = function(self) end

function _proto:clone(mixin)
	local new = { __index = self }
	setmetatable(new, new)

	new:__clone()

	if mixin then
		table.merge(new, mixin)
	end

	return new
end

-- Simple Self/iolang style prototype chains
-- Can be used with lua serialization as long as no functions are set anywhere
-- but on the base prototype returned from utils.proto
utils.proto = function(classname)
	local newProto = _proto:clone()

	newProto.class = classname

	function newProto:Serialize()
		local out = table.copy(self)

		-- Cannot serialize functions, so references to the base prototype are
		-- not serialized
		if out.__index == newProto then
			out.__index = nil
		end

		return out
	end

	-- If a prototype doesn't have a serialized __index field, it referred to
	-- this base prototype originally
	function newProto:Unserialize()
		self.__index = self.__index or newProto
		return setmetatable(self, self)
	end

	return newProto
end

--
-- Function: print_r
--
-- Recursively print the contents of a table to the console.
--
utils.print_r = function(t)
	local print_r_cache={}
	local print_str_cache = {}

	local write = function(indent, str, ...)
		table.insert(print_str_cache, string.rep(" ", indent) .. string.format(str, ...))
	end

	local function sub_print_r(t, indent)
		if (print_r_cache[tostring(t)]) then
			write(indent, "*" .. tostring(t))
		else
			print_r_cache[tostring(t)] = true

			if type(t) == "table" then
				for key, val in pairs(t) do
					local string_key = tostring(key)
					local string_val = tostring(val)

					if type(val) == "table" and not print_r_cache[string_val] then
						write(indent, '[%s] => %s {', string_key, string_val)

						sub_print_r(val, indent + string.len(string_key) + 8)

						write(indent + string.len(string_key) + 6, "}")
					elseif type(val) == "table" then
						write(indent, "[%s] => *%s", string_key, string_val)
					elseif (type(val)=="string") then
						write(indent, "[%s] => '%s'", string_key, string_val)
					else
						write(indent, "[%s] => %s", string_key, string_val)
					end
				end
			else
				write(indent, "%s", tostring(t))
			end
		end
	end

	if (type(t)=="table") then
		write(0, "%s {", tostring(t))
		sub_print_r(t, 2)
		write(0, "}")
	else
		sub_print_r(t, 2)
	end

	print(table.concat(print_str_cache, "\n"))
end

--
-- Function: count
--
-- Count the number of entries in a table
--
utils.count = function(t)
	local i = 0
	for _,_ in pairs(t) do
		i = i + 1
	end
	return i
end

--
-- Function: contains
--
-- Return true if the function contains the given value under any key.
--
utils.contains = function(t, val)
	for _, v in pairs(t) do
		if v == val then return true end
	end

	return false
end

--
-- Function: utils.indexOf
--
-- looking for the first counter index of an element in an array.
--
-- Example:
--
-- > local index = utils.indexOf(ShipType.shipIDs, shipID)
--
-- Parameters:
--
--   array
--
--   value - searched array element
--
-- Return:
--
--   value - any, array item
--
utils.indexOf = function(array, value)
	for i, v in ipairs(array) do
		if v == value then
			return i
		end
	end
end

--
-- Function: remove_elem
--
-- Remove the given value element from the passed array table
--
-- Returns the index that the element was removed from or nil if it wasn't removed
utils.remove_elem = function(t, val)
	for i = #t, 1, -1 do
		if t[i] == val then
			table.remove(t, i)
			return i
		end
	end
	return nil
end

--
-- Function: take
--
-- Return an iterator that iterates through the first N values of the given array table.
--
utils.take = function(t, n, skip)
	local f = function(s, k)
		k = k + 1
		if k > n or s[k] == nil then return nil end
		return k, s[k]
	end
	return f, t, skip or 0
end

--
-- Function: reverse
--
-- Return an iterator that iterates backwards through the given array table.
--
utils.reverse = function(t, start)
	local f = function(s, k)
		k = k - 1
		if k <= 0 then return end
		return k, s[k]
	end
	return f, t, (start or #t) + 1
end

--
-- Function: round
--
-- Round any real number, x, to closest multiple of magnitude |N|,
-- but never lower. N defaults to 1, if omitted.
--
-- Example:
--   > x_steps_of_N = round(x, N)
--
utils.round = function(x, n)
	local s = math.sign(x)
	n = n or 1
	n = math.abs(n)
	x = math.round(math.abs(x)/n)*n
	return x < n and n*s or x*s
end

--
-- Function: utils.deviation
--
-- Returns a random value that differs from nominal by no more than nominal * ratio.
--
-- value = utils.deviation(nominal, ratio)
--
-- Return:
--
--   value - number
--
-- Parameters:
--
--   nominal - number
--   ratio - number, indicating the relative deviation of the result
--
-- Example:
--
-- > value = utils.deviation(100, 0.2) -- 80 < value < 120
--
utils.deviation = function(nominal, ratio)
	return nominal * Engine.rand:Number(1 - ratio, 1 + ratio)
end

--
-- Function: utils.asymptote
--
-- The function is used to limit the value of the argument, but softly.
-- See the desctription of the arguments.
--
-- value = utils.asymptote(x, max_value, equal_to_ratio)
--
-- Return:
--
--   value - number
--
-- Parameters:
--
--   x - number
--   max_value - the return value will never be greater than this number, it
--               will asymptotically approach it
--   equal_to_ratio - 0.0 .. 1.0, the ratio between x and max_value up to which x is returned as is
--
-- Example:
--
-- > value = utils.asymptote(10,    100, 0.5) -- return 10
-- > value = utils.asymptote(70,    100, 0.5) -- return 64.285
-- > value = utils.asymptote(700,   100, 0.5) -- return 96.428
-- > value = utils.asymptote(70000, 100, 0.5) -- return 99.964
--
utils.asymptote = function(x, max_value, equal_to_ratio)
	local equal_to = max_value * equal_to_ratio
	local equal_from = max_value - equal_to
	if x < equal_to then
		return x
	else
		return (1 - 1 / ((x - equal_to) / equal_from + 1)) * equal_from + equal_to
	end
end

--
-- Function: utils.normWeights
--
-- the input is an array of hashtables with an arbitrary real number in the
-- weight key. Weights are recalculated so that the sum of the weights in the
-- entire array equals 1 in fact, now these are the probabilities of selecting
-- an item in the array
--
-- Example:
--
-- > utils.normWeights({ {param = 10, weight = 3.4},
-- >                       {param = 15, weight = 2.1} })
--
-- Parameters:
--
--   array - an array of similar hashtables with an arbitrary real number in
--           the weight key
--
-- Returns:
--
--  nothing
--
utils.normWeights = function(array)
	local sum = 0
	for _,v in ipairs(array) do
		sum = sum + v.weight
	end
	for _,v in ipairs(array) do
		v.weight = v.weight / sum
	end
end

--
-- Function: utils.chooseNormalized
--
-- Choose random item, considering the weights (probabilities).
-- Each array[i] should have 'weight' key.
-- The sum of the weights must be equal to 1.
--
-- Example:
--
-- > my_param = utils.chooseNormalized({ {param = 10, weight = 0.62},
-- >                                       {param = 15, weight = 0.38} }).param
--
-- Parameters:
--
--   array - an array of hashtables with an arbitrary real number in
--           the weight key
--   rand  - optional, Engine.rand object
--
-- Returns:
--
--   a random element of the array, with the probability specified in the weight key
--
utils.chooseNormalized = function(array, rand)
	if not rand then rand = Engine.rand end
	local choice, sum = rand:Number(1.0), 0.0
	for _, option in ipairs(array) do
		sum = sum + option.weight
		if choice <= sum then return option end
	end
end

--
-- Function: utils.chooseEqual
--
-- Returns a random element of an array
--
-- Example:
--
-- > my_param = utils.chooseEqual({ {param = 10},
-- >                                  {param = 15} }).param
--
-- Parameters:
--
--   array - an array of hashtables
--   rand  - optional, Engine.rand object
--
-- Returns:
--
--   a random element of the array, with the with equal probability for any element
--
utils.chooseEqual = function(array, rand)
	if not rand then rand = Engine.rand end
	return array[rand:Integer(1, #array)]
end

--
-- Function: utils.getIndexFromIntervals
--
-- Searches for the index of an element from an array of intervals. Each array
-- element is itself an array of two elements, the first is the return value
-- and the second is the interval. The value lies in the interval from its
-- number and above.
--
-- Example:
--
-- >    reputations = {
-- >        { 'INCOMPETENT'        },
-- >        { 'UNRELIABLE',     -8 },
-- >        { 'NOBODY',          0 },
-- >        { 'INEXPERIENCED',   4 },
-- >        { 'EXPERIENCED',     8 },
-- >        { 'CREDIBLE',       16 },
-- >        { 'RELIABLE',       32 },
-- >        { 'TRUSTWORTHY',    64 },
-- >        { 'PROFESSIONAL',  128 },
-- >        { 'EXPERT',        256 },
-- >        { 'MASTER',        512 }
-- >    },
-- >
-- > reputation = utils.getIndexFromIntervals(reputations, 42)
-- > assert(reputation == 7)
--
-- Parameters:
--
--   array - sorted array of intervals
--   number - some value in intervals,
--
-- Returns:
--
--  number - index
--
utils.getIndexFromIntervals = function(array, value)
	for i = #array, 1, -1 do
		if not array[i][2] or value >= array[i][2] then
			return i
		end
	end
	assert(false, "array of intervals is not valid!")
end

--
-- Function: utils.getFromIntervals
--
-- Searches for the  element from an array of intervals. Similar to
-- getIndexFromIntervals but returns the value at the index.
--
-- Example:
--
-- > reputation = utils.getFromIntervals(reputations, 42) -- see getIndexFromIntervals
-- > assert(reputation == 'RELIABLE')
--
-- Parameters:
--
--   array - sorted array of intervals
--   number - some value in intervals
--
-- Returns:
--
--  value - any, what is contained in the array (without a number-interval)
--
utils.getFromIntervals = function(array, value)
	return array[utils.getIndexFromIntervals(array, value)][1]
end

return utils
