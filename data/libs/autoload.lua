-- Copyright © 2008-2024 Pioneer Developers. See AUTHORS.txt for details
-- Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

-- this is the only library automatically loaded at startup
-- its the right place to extend core Lua tables

string.trim = function(s)
	return string.gsub(s or "", "^%s*(.-)%s*$", "%1")
end

math.round = function(v)
	return (math.modf(v + (v < 0.0 and -.5 or .5)))
end

math.sign = function(v)
	return (v > 0 and 1) or (v == 0 and 0) or -1
end

math.clamp = function(v, min, max)
	return math.min(max, math.max(v,min))
end

-- linearly interpolate between min and max according to V
math.lerp = function(min, max, v)
	return min + (max - min) * v
end

-- calculate the interpolation factor of the given number v relative to min and max
math.invlerp = function(min, max, v)
	return (v - min) / (max - min)
end

debug.deprecated = function(name)
	local deprecated_function = debug.getinfo(2)
	local caller = debug.getinfo(3)
	local ending = ""
	if caller then
		ending = "The call originated at <"..caller.source..":"..caller.currentline..">."
	else
		ending = "It is not possible to retrace the caller."
	end
	print("The function \""..(name or deprecated_function.name or "[unknown]").."\" has changed its interface or is deprecated. "..ending)

	print("Please check the changelogs and/or get in touch with the development team.")
end

-- a nice string interpolator
string.interp = function (s, t)
	local i = 0
	return (s:gsub('(%b{})', function(w)
		if #w > 2 then
			return tostring(t[w:sub(2, -2)]) or w
		else
			i = i + 1; return tostring(t[i]) or w
		end
	end))
end

---@class string
---@operator mod(table): string

-- allow using string.interp via "s" % { t }
getmetatable("").__mod = string.interp

-- make a simple shallow copy of the passed-in table
-- does not copy metatable nor recurse into the table
---@generic T
---@param t T
---@return T
table.copy = function(t)
	local ret = {}
	for k, v in pairs(t) do
		ret[k] = v
	end
	return ret
end

-- Copy values from table b into a
--
-- Does not copy metatable nor recurse into the table.
-- Pass an optional predicate to transform the keys and values before assignment.
---@generic K, V
---@param a table
---@param b table<K, V>
---@param predicate nil|fun(k: K, v: V): any, any
---@return table
table.merge = function(a, b, predicate)
	for k, v in pairs(b) do
		if predicate then k, v = predicate(k, v) end
		a[k] = v
	end
	return a
end

-- Append array b to array a
--
-- Does not copy metatable nor recurse into the table.
-- Pass an optional predicate to transform the keys and values before assignment.
---@generic T
---@param a table
---@param b T[]
---@param predicate nil|fun(v: T): any
---@return table
table.append = function(a, b, predicate)
	for _, v in ipairs(b) do
		if predicate then v = predicate(v) end
		table.insert(a, v)
	end
	return a
end

-- make import break. you should never import this file
return nil
