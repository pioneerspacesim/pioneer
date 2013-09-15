-- Copyright © 2008-2013 Pioneer Developers. See AUTHORS.txt for details
-- Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

-- this is the only library automatically loaded at startup
-- its the right place to extend core Lua tables

math.clamp = function(v, min, max)
	return math.min(max, math.max(v,min))
end

debug.deprecated = function()
	local deprecated_function = debug.getinfo(2)
	local caller = debug.getinfo(3)
	print("The use of the function \""..deprecated_function.name.."\" as done at <"..caller.source..":"..caller.currentline.."> is deprecated")
	print("Please check the changelogs and/or get in touch with the development team.")
end

-- a nice string interpolator
string.interp = function (s, t)
	return (s:gsub('(%b{})', function(w) return t[w:sub(2,-2)] or w end))
end

-- make import break. you should never import this file
return nil
