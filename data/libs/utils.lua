-- Copyright © 2008-2017 Pioneer Developers. See AUTHORS.txt for details
-- Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

local utils
utils = {}

--
-- numbered_keys: transform an iterator to one that returns numbered keys
--
-- for k,v in numbered_keys(pairs(table)) do ... end
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
-- filter: transform an iterator to one that only returns items that
--         match some predicate
--
-- for k,v in filter(function (k,v) ... return true end, pairs(table))
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
-- map: transform an iterator to one that returns modified keys/values
--
-- for k,v in map(function (k,v) ... return newk, newv end, pairs(table))
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
-- build_array: return a table containing all values returned by an iterator
--              returned table is built using table.insert (integer keys)
--
-- array = build_array(pairs(table))
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
-- build_table: return a table containing all values returned by an iterator
--              returned table is build using t[k] = v
--
-- filtered = build_table(filter(function () ... end, pairs(table)))
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
-- stable_sort: return a sorted table. Sort isn't fast but stable
-- (default Lua table.sort is fast and unstable).
-- stable_sort uses Merge sort algorithm.
--
-- sorted_table = stable_sort(unsorted_table,
--							  function (a,b) return a < b end)
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

	function merge_sort (values)
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

--
-- inherits(baseClass): returns a new class that implements inheritage from
-- the provided base class.
--
-- To overwrite the constructor (function `New`), don't forget to rename the current
-- one and call it in the new method.
--
local object = {}

object.meta = { __index = object, class="object" }

function object.New(args)
	local newinst = {}
	setmetatable( newinst, object.meta )
	return newinst
end

function object:Serialize()
	return self
end

function object.Unserialize(data)
	setmetatable(data, object.meta)
	return data
end

utils.inherits = function (baseClass, name)
	local new_class = {}
	local base_class = baseClass or object
	new_class.meta = { __index = new_class, class=name }

	-- generic constructor
	function new_class.New(args)
		local newinst = baseClass.New(args)
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

utils.print_r = function(t)
	local print_r_cache={}
	local function sub_print_r(t,indent,rec_guard)
		if (print_r_cache[tostring(t)]) then
			print(indent.."*"..tostring(t))
		else
			print_r_cache[tostring(t)]=true
			if (type(t)=="table") then
				for pos,val in pairs(t) do
					local string_pos = tostring(pos)
					if (type(val)=="table") then
						print(indent.."["..string_pos.."] => "..tostring(t).." {")
						sub_print_r(val,indent..string.rep(" ",string.len(string_pos)+8))
						print(indent..string.rep(" ",string.len(string_pos)+6).."}")
					elseif (type(val)=="string") then
						print(indent.."["..string_pos..'] => "'..val..'"')
					else
						print(indent.."["..string_pos.."] => "..tostring(val))
					end
				end
			else
				print(indent..tostring(t))
			end
		end
	end
	if (type(t)=="table") then
		print(tostring(t).." {")
		sub_print_r(t,"  ")
		print("}")
	else
		sub_print_r(t,"  ")
	end
	print()
end

-- Count the number of entries in a table
utils.count = function(t)
	local i = 0
	for _,v in pairs(t) do
		i = i + 1
	end
	return i
end

utils.take = function(t, n)
	local res = {}
	local i = 0
	for _,v in pairs(t) do
		if i == n then
			return res
		else
			table.insert(res, v)
			i = i + 1
		end
	end
	return res
end

utils.reverse = function(t)
	local res = {}
	for _,v in pairs(t) do
		table.insert(res, 1, v)
	end
	return res
end
return utils
