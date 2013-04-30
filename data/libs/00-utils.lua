-- Copyright © 2008-2013 Pioneer Developers. See AUTHORS.txt for details
-- Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

math.clamp = function(v, min, max)
	return math.min(max, math.max(v,min))
end

debug.deprecated = function()
	local deprecated_function = debug.getinfo(2)
	local caller = debug.getinfo(3)
	print("The use of the function \""..deprecated_function.name.."\" as done at <"..caller.source..":"..caller.currentline.."> is deprecated")
	print("Please check the changelogs and/or get in touch with the development team.")
end


--
-- numbered_keys: transform an iterator to one that returns numbered keys
--
-- for k,v in numbered_keys(pairs(table)) do ... end
--
function numbered_keys(step, context, position)
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
function filter(predicate, step, context, position)
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
function map(transformer, step, context, position)
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
function build_array(f, s, k)
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
function build_table(f, s, k)
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

function stable_sort(values, cmp)
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
