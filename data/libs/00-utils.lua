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
